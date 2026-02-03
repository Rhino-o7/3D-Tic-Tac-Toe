#include "Application.h"
#include <imgui.h>
#include <iostream>
#ifndef __EMSCRIPTEN__
    #include <thread>
#endif

Application::Application(int windowWidth, int windowHeight) :
    m_Renderer((float)windowWidth / (float)windowHeight),
    m_SelectedChunkIndex(0),
    m_NeedsUpdate(false),
    m_HoveredChunkIndex(std::nullopt),
    m_GameState(GameState::NotConnected),
    m_ServerAddress("ws://localhost:9002"),
    m_ConnectionStatus("Not connected"),
    m_Winner(Player::NONE),
	m_WindowWidth(windowWidth),
	m_WindowHeight(windowHeight)
{
    m_Builder.BuildChunks();
    m_Builder.BuildChunksMesh();

    m_Renderer.LoadChunks(
        m_Builder.GetChunkMeshes(),
        m_Builder.GetChunkOffsets(),
        m_Builder.GetChunkSize());
    
    // Set callback for game state changes
    m_VoxelClient.SetOnStateChangeCallback([this]() {
       m_NeedsUpdate.store(true, std::memory_order_release);
    });
}

Application::~Application() {
    // Clear hover
    if (m_HoveredChunkIndex.has_value()) {
        if (Chunk* chunk = m_Builder.GetChunk(m_HoveredChunkIndex.value())) {
            chunk->ClearChunkHover();
        }
    }

#ifndef __EMSCRIPTEN__

    m_VoxelClient.StopGameLoop();
    if (m_GameThread.joinable()) {
        m_GameThread.join();
    }
#endif
}

void Application::SetGameState(GameState state)
{
	m_GameState.store(state, std::memory_order_release);
}

Application::GameState Application::GetGameState() const
{
	return m_GameState.load(std::memory_order_acquire);
}

void Application::SetConnectionStatus(const std::string& status)
{
	std::lock_guard<std::mutex> lock(m_StatusMutex);
	m_ConnectionStatus = status;
}

std::string Application::GetConnectionStatus() const
{
	std::lock_guard<std::mutex> lock(m_StatusMutex);
	return m_ConnectionStatus;
}

void Application::OnUpdate(float deltaTime)
{
	m_Renderer.OnUpdate(deltaTime);
	
	// Check for disconnection
	GameState currentState = GetGameState();
	if (currentState == GameState::WaitingForPlayerChoice || 
	    currentState == GameState::Playing || 
	    currentState == GameState::GameOver) {
		CheckDisconnection();
	}
	
	if (currentState == GameState::Playing) {
		CheckGameOver();
		
		UpdateHoverState();
		
		HandleMouseInput();
	}
	
	// Update chunks 
	if (m_NeedsUpdate) {
		UpdateChunksFromBoard();
		m_NeedsUpdate = false;
	}
}

void Application::ConnectToServer()
{
    SetGameState(GameState::Connecting);
    SetConnectionStatus("Connecting...");

#ifdef __EMSCRIPTEN__
    // Web build: Connection is async
    if (m_VoxelClient.connectToServer(m_ServerAddress)) {
        // Connection initiated
        SetGameState(GameState::WaitingForPlayerChoice);
        SetConnectionStatus("Connected! Choose your player.");
    } else {
        SetGameState(GameState::NotConnected);
        SetConnectionStatus("Failed to connect to server.");
    }
#else
    std::thread connectionThread([this]() {
        // Connect to server 
        if (m_VoxelClient.connectToServer(m_ServerAddress)) {
            // Start game loop on thread
            m_GameThread = std::thread([this]() {
                m_VoxelClient.StartGameLoop();
            });

            SetGameState(GameState::WaitingForPlayerChoice);
            SetConnectionStatus("Connected! Choose your player.");
        } else {
            SetGameState(GameState::NotConnected);
            SetConnectionStatus("Failed to connect to server.");
        }
    });
    connectionThread.detach();
#endif
}

void Application::DisconnectFromServer()
{
#ifndef __EMSCRIPTEN__
    m_VoxelClient.StopGameLoop();

    if (m_GameThread.joinable()) {
        m_GameThread.join();
    }
#endif

    // Rebuild chunks
    m_Builder.BuildChunks();
    m_Builder.BuildChunksMesh();

    // Reload renderer
    m_Renderer.LoadChunks(
        m_Builder.GetChunkMeshes(),
        m_Builder.GetChunkOffsets(),
        m_Builder.GetChunkSize());

    // Reset state
    SetGameState(GameState::NotConnected);
    SetConnectionStatus("Not connected");
    m_Winner = Player::NONE;
    m_HoveredChunkIndex = std::nullopt;
}

void Application::CheckDisconnection()
{
	if (!m_VoxelClient.IsConnected()) {
		std::cout << "Server disconnected! Returning to menu..." << std::endl;
		
		// Clear hover 
		if (m_HoveredChunkIndex.has_value()) {
			if (Chunk* chunk = m_Builder.GetChunk(m_HoveredChunkIndex.value())) {
				chunk->ClearChunkHover();
				RebuildAndUploadChunk(m_HoveredChunkIndex.value());
			}
			m_HoveredChunkIndex = std::nullopt;
		}
		
		DisconnectFromServer();
		SetConnectionStatus("Server disconnected.");
	}
}

void Application::CheckGameOver()
{
	if (!m_VoxelClient.IsConnected()) {
		return;
	}
	
	GameStateData gameState = m_VoxelClient.GetLastGameState();
	
	if (gameState.game_over) {
		SetGameState(GameState::GameOver);
		m_Winner = gameState.winner;
		
		// Clear hover 
		if (m_HoveredChunkIndex.has_value()) {
			if (Chunk* chunk = m_Builder.GetChunk(m_HoveredChunkIndex.value())) {
				chunk->ClearChunkHover();
				RebuildAndUploadChunk(m_HoveredChunkIndex.value());
			}
			m_HoveredChunkIndex = std::nullopt;
		}
		
		// Update connection 
		if (m_Winner == Player::NONE) {
			SetConnectionStatus("Game Over - Draw!");
		} else {
			SetConnectionStatus("Game Over - Player " + std::string(m_Winner == Player::X ? "X" : "O") + " wins!");
		}
	}
}

void Application::UpdateHoverState()
{
	ImGuiIO& io = ImGui::GetIO();
	
	if (io.WantCaptureMouse) {
		if (m_HoveredChunkIndex.has_value()) {
			if (Chunk* chunk = m_Builder.GetChunk(m_HoveredChunkIndex.value())) {
				chunk->ClearChunkHover();
				RebuildAndUploadChunk(m_HoveredChunkIndex.value());
			}
			m_HoveredChunkIndex = std::nullopt;
		}
		return;
	}
	
	// mouse position
	double mouseX = io.MousePos.x;
	double mouseY = io.MousePos.y;
	
	// Create ray from camera 
	Ray ray = ScreenToWorldRay(mouseX, mouseY, m_WindowWidth, m_WindowHeight);
	RayHit hit = RaycastChunks(ray);
	
	std::optional<std::size_t> newHoveredIndex = hit.hit ? std::optional<std::size_t>(hit.chunkIndex) : std::nullopt;
	
	// Check if hover state changed
	if (newHoveredIndex != m_HoveredChunkIndex) {
		// Clear previous
		if (m_HoveredChunkIndex.has_value()) {
			if (Chunk* chunk = m_Builder.GetChunk(m_HoveredChunkIndex.value())) {
				chunk->ClearChunkHover();
				RebuildAndUploadChunk(m_HoveredChunkIndex.value());
			}
		}
		
		// Set hover
		if (newHoveredIndex.has_value()) {
			if (Chunk* chunk = m_Builder.GetChunk(newHoveredIndex.value())) {
				chunk->SetChunkHover();
				RebuildAndUploadChunk(newHoveredIndex.value());
			}
		}
		
		m_HoveredChunkIndex = newHoveredIndex;
	}
}

void Application::HandleMouseInput()
{
	ImGuiIO& io = ImGui::GetIO();
	
	if (io.WantCaptureMouse) {
		return;
	}
	
	bool currentRightMouseState = ImGui::IsMouseDown(ImGuiMouseButton_Left);
	
	// Click 
	if (currentRightMouseState && !m_LastRightMouseState) {
		double mouseX = io.MousePos.x;
		double mouseY = io.MousePos.y;
		
		// Create ray from camera 
		Ray ray = ScreenToWorldRay(mouseX, mouseY, m_WindowWidth, m_WindowHeight);
		RayHit hit = RaycastChunks(ray);
		
		if (hit.hit) {
			// Send move to the server
			if (m_VoxelClient.SendMove(hit.chunkCoords.x, hit.chunkCoords.y, hit.chunkCoords.z)) {
				std::cout << "Raycast hit chunk at (" 
				          << hit.chunkCoords.x << ", " 
				          << hit.chunkCoords.y << ", " 
				          << hit.chunkCoords.z << ")\n";
			} else {
				std::cout << "Hit chunk but not your turn or unable to send move.\n";
			}
		}
	}
	
	m_LastRightMouseState = currentRightMouseState;
}

// Convert screen coordinates to a world ray
Application::Ray Application::ScreenToWorldRay(double mouseX, double mouseY, int screenWidth, int screenHeight)
{
	const Camera& camera = m_Renderer.GetCamera();
	
	// Convert screen coordinates to normalized device coordinates
	float ndcX = (2.0f * mouseX) / screenWidth - 1.0f;
	float ndcY = 1.0f - (2.0f * mouseY) / screenHeight;
	
	// Create NDC point at near plane
	glm::vec4 rayClip(ndcX, ndcY, -1.0f, 1.0f);
	
	// Transform to eye space
	glm::mat4 projectionMatrix = camera.GetProjectionMatrix();
	glm::vec4 rayEye = glm::inverse(projectionMatrix) * rayClip;
	rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
	
	// Transform to world space
	glm::mat4 viewMatrix = camera.GetViewMatrix();
	glm::vec4 rayWorld = glm::inverse(viewMatrix) * rayEye;
	glm::vec3 rayDirection = glm::normalize(glm::vec3(rayWorld));
	
	Ray ray;
	ray.origin = camera.GetPosition();
	ray.direction = rayDirection;
	
	return ray;
}

// Get the closest chunk hit by the ray
Application::RayHit Application::RaycastChunks(const Ray& ray)
{
	RayHit closestHit;
	closestHit.hit = false;
	closestHit.distance = std::numeric_limits<float>::max();
	
	const glm::uvec3 gridSize = m_Builder.GetChunkGridSize();
	const glm::ivec3 chunkSize = m_Builder.GetChunkSize();
	
	GameStateData gameState = m_VoxelClient.GetLastGameState();
	bool hasGameState = m_VoxelClient.IsConnected();
	
	float closestDistanceToRay = std::numeric_limits<float>::max();
	
	// Test all chunks in the grid
	for (int x = 0; x < static_cast<int>(gridSize.x); ++x) {
		for (int y = 0; y < static_cast<int>(gridSize.y); ++y) {
			for (int z = 0; z < static_cast<int>(gridSize.z); ++z) {
				// Check for empty chunk
				bool isEmpty = false;
				if (hasGameState && x >= 0 && x < 3 && y >= 0 && y < 3 && z >= 0 && z < 3) {
					isEmpty = (gameState.board[x][y][z] == Player::NONE);
				} else if (!hasGameState) {
					isEmpty = true; 
				}
				
				if (!isEmpty) continue;
				
				std::size_t chunkIndex = x + y * gridSize.x + z * gridSize.x * gridSize.y;				
				const glm::vec3* offset = m_Builder.GetChunkOffset(chunkIndex);

				if (!offset) continue;

				glm::vec3 chunkCenter = *offset + glm::vec3(chunkSize) * 0.5f;
				
				// Calculate distance from chunk center to ray line
				glm::vec3 AP = chunkCenter - ray.origin;
				float projection = glm::dot(AP, ray.direction);
				
				// Only consider chunks in front of the camera
				if (projection < 0.0f) continue;
				
				glm::vec3 closestPointOnRay = ray.origin + ray.direction * projection;
				float distanceToRay = glm::length(chunkCenter - closestPointOnRay);
				
				// Check if this is the closest chunk to the ray
				if (distanceToRay < closestDistanceToRay) {
					closestDistanceToRay = distanceToRay;
					closestHit.hit = true;
					closestHit.distance = projection; 
					closestHit.chunkCoords = glm::ivec3(x, y, z);
					closestHit.chunkIndex = chunkIndex;
				}
			}
		}
	}
	
	return closestHit;
}

bool Application::RayAABBIntersection(const Ray& ray, const glm::vec3& aabbMin, const glm::vec3& aabbMax, float& tMin)
{
	float tMinCalc = 0.0f;
	float tMaxCalc = std::numeric_limits<float>::max();
	
	for (int i = 0; i < 3; ++i) {
		if (abs(ray.direction[i]) < 1e-8f) {
			// Ray is parallel to slab: check if origin is within bounds
			if (ray.origin[i] < aabbMin[i] || ray.origin[i] > aabbMax[i]) {
				return false;
			}
		} else {
			// Compute intersection t values with near and far planes
			float t1 = (aabbMin[i] - ray.origin[i]) / ray.direction[i];
			float t2 = (aabbMax[i] - ray.origin[i]) / ray.direction[i];
			
			if (t1 > t2) std::swap(t1, t2);
			
			tMinCalc = std::max(tMinCalc, t1);
			tMaxCalc = std::min(tMaxCalc, t2);
			
			if (tMinCalc > tMaxCalc) {
				return false;
			}
		}
	}
	
	tMin = tMinCalc;
	return tMin >= 0.0f;
}

void Application::OnRender()
{
	m_Renderer.OnRender();
}

void Application::OnImGuiRender()
{
	//m_Renderer.OnImGuiRender();

	ImGui::Begin("3D Tic-Tac-Toe");
	
	GameState currentState = GetGameState();
	std::string currentStatus = GetConnectionStatus();
	
	// Connection UI
	if (currentState == GameState::NotConnected) {
		ImGui::SeparatorText("Connect to Server");
		
		// Show status message
		if (currentStatus != "Not connected") {
			ImVec4 color = (currentStatus.find("Failed") != std::string::npos || 
			                currentStatus.find("disconnected") != std::string::npos) 
			                ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) 
			                : ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
			ImGui::TextColored(color, "%s", currentStatus.c_str());
			ImGui::Spacing();
		}
		
		ImGui::Text("Enter server address:");
		
		// Reserve space for input and use .data() as mutable buffer
		m_ServerAddress.reserve(256);
		if (ImGui::InputText("##ServerAddress", m_ServerAddress.data(), m_ServerAddress.capacity() + 1)) {
			// Update string size after input
			m_ServerAddress.resize(std::strlen(m_ServerAddress.data()));
		}
		
		ImGui::Spacing();
		
		if (ImGui::Button("Connect", ImVec2(200, 40))) {
			ConnectToServer();
		}
		
		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Example: ws://localhost:9002");
	}
	// Player Choice UI
	else if (currentState == GameState::WaitingForPlayerChoice) {
		ImGui::SeparatorText("Choose Your Player");
		
		ImGui::Text("%s", currentStatus.c_str());
		ImGui::Spacing();
		ImGui::Spacing();
		
		ImGui::Text("Select X or O:");
		ImGui::Spacing();
		
		if (ImGui::Button("Play as X", ImVec2(200, 60))) {
			m_VoxelClient.SetPlayerChoice(Player::X);
			SetGameState(GameState::Playing);
			SetConnectionStatus("Playing as X");
		}
		
		ImGui::Spacing();
		
		if (ImGui::Button("Play as O", ImVec2(200, 60))) {
			m_VoxelClient.SetPlayerChoice(Player::O);
			SetGameState(GameState::Playing);
			SetConnectionStatus("Playing as O");
		}
	}
	// Game Over UI
	else if (currentState == GameState::GameOver) {
		ImGui::SeparatorText("Game Over");
		
		ImGui::Spacing();
		ImGui::Spacing();
		
		// Display winner 
		if (m_Winner == Player::NONE) {
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "It's a Draw!");
		} else if (m_Winner == Player::X) {
			ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Player X Wins!");
		} else {
			ImGui::TextColored(ImVec4(0.3f, 0.6f, 1.0f, 1.0f), "Player O Wins!");
		}
		
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		
		// Play Again button
		if (ImGui::Button("Play Again", ImVec2(250, 70))) {
			DisconnectFromServer();
			ConnectToServer();
		}
		
		ImGui::Spacing();
		
		// Return to Menu button
		if (ImGui::Button("Return to Menu", ImVec2(250, 50))) {
			DisconnectFromServer();
		}
	}
	// Game UI
	else if (currentState == GameState::Playing) {
		ImGui::SeparatorText("Game Status");
		
		ImGui::Text("Status: %s", currentStatus.c_str());
		ImGui::Text("Connected: %s", m_VoxelClient.IsConnected() ? "Yes" : "No");
		ImGui::Text("Your Turn: %s", m_VoxelClient.IsWaitingForTurn() ? "Yes" : "No");
		
		ImGui::SeparatorText("Controls");
		
		ImGui::Text("Hover over a chunk to preview");
		ImGui::Text("Right-click on a chunk to make move");
		
		ImGui::Spacing();
		
		if (m_HoveredChunkIndex.has_value()) {
			const glm::uvec3 gridSize = m_Builder.GetChunkGridSize();
			std::size_t idx = m_HoveredChunkIndex.value();
			int x = idx % gridSize.x;
			int y = (idx / gridSize.x) % gridSize.y;
			int z = idx / (gridSize.x * gridSize.y);
			ImGui::Text("Hovering: (%d, %d, %d)", x, y, z);
		} else {
			ImGui::TextDisabled("Hovering: None");
		}
	}
	// Connecting UI
	else if (currentState == GameState::Connecting) {
		ImGui::SeparatorText("Connecting");
		ImGui::Text("%s", currentStatus.c_str());
		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Please wait...");
	}
	
	ImGui::End();
}

void Application::UpdateChunksFromBoard()
{
	std::vector<glm::vec3> updatedChunks = m_VoxelClient.GetUpdatedChunks();
	
	if (updatedChunks.empty()) {
		return;
	}
	
	GameStateData gameState = m_VoxelClient.GetLastGameState();
	const glm::uvec3& gridSize = m_Builder.GetChunkGridSize();

	for (const glm::vec3& chunkPos : updatedChunks) {
		int x = static_cast<int>(chunkPos.x);
		int y = static_cast<int>(chunkPos.y);
		int z = static_cast<int>(chunkPos.z);

		// error check
		if (x < 0 || x >= 3 || y < 0 || y >= 3 || z < 0 || z >= 3) {
			continue;
		}
		std::size_t chunkIndex = x + y * gridSize.x + z * gridSize.x * gridSize.y;
		Chunk* chunk = m_Builder.GetChunk(chunkIndex);

		if (chunk) {
			// Update chunk
			Player cellPlayer = gameState.board[x][y][z];
			chunk->SetChunkPlayer(cellPlayer);
			RebuildAndUploadChunk(chunkIndex);
		}
	}
}

void Application::RebuildAndUploadChunk(std::size_t index)
{
	if (!m_Builder.RebuildChunkMesh(index)) {
		return;
	}

	if (const Mesh* mesh = m_Builder.GetChunkMesh(index)) {
		m_Renderer.UpdateChunkMesh(index, *mesh);
	}
}

void Application::FillChunk(Chunk& chunk, VoxelType type)
{
	for (int x = 0; x < chunk.GetSizeX(); ++x) {
		for (int y = 0; y < chunk.GetSizeY(); ++y) {
			for (int z = 0; z < chunk.GetSizeZ(); ++z) {
				chunk.SetIndex(x, y, z, type);
			}
		}
	}
}

void Application::OnResize(int width, int height)
{
    m_WindowWidth = width;
    m_WindowHeight = height;

    // Update renderer aspect ratio
    float aspect = (height != 0) ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
    m_Renderer.SetAspectRatio(aspect);

    glViewport(0, 0, width, height);
}