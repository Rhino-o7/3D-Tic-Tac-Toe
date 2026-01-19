#include "TestChunk.h"
#include <imgui.h>
namespace Test {

	TestChunk::TestChunk()
		: m_Renderer(2000.0f / 1000.0f),
		m_SelectedChunkIndex(0),
		m_NeedsUpdate(false)
	{
		m_Builder.BuildChunks();
		m_Builder.BuildChunksMesh();

		m_Renderer.LoadChunks(
			m_Builder.GetChunkMeshes(),
			m_Builder.GetChunkOffsets(),
			m_Builder.GetChunkSize());
		
		// Set callback for when game state changes
		m_VoxelClient.SetOnStateChangeCallback([this]() {
			m_NeedsUpdate = true;
		});
		
		// Connect to server
		if (m_VoxelClient.connectToServer("ws://localhost:9002")) {
			// Start game loop on separate thread
			m_GameThread = std::thread([this]() {
				m_VoxelClient.StartGameLoop();
			});
		}
	}
	
	TestChunk::~TestChunk() {
		// Wait for game thread to finish
		if (m_GameThread.joinable()) {
			m_GameThread.join();
		}
	}

	void TestChunk::OnUpdate(float deltaTime)
	{
		m_Renderer.OnUpdate(deltaTime);
		
		// Automatically update chunks when state changes
		if (m_NeedsUpdate) {
			UpdateChunksFromBoard();
			m_NeedsUpdate = false;
		}
	}

	void TestChunk::OnRender()
	{
		m_Renderer.OnRender();
	}

	void TestChunk::OnImGuiRender()
	{
		m_Renderer.OnImGuiRender();

		ImGui::SeparatorText("Network Status");
		
		ImGui::Text("Connected: %s", m_VoxelClient.IsConnected() ? "Yes" : "No");
		ImGui::Text("Your Turn: %s", m_VoxelClient.IsWaitingForTurn() ? "Yes" : "No");
		
		ImGui::SeparatorText("Player Selection");
		
		if (ImGui::Button("Play as X")) {
			m_VoxelClient.SetPlayerChoice(Player::X);
		}
		ImGui::SameLine();
		if (ImGui::Button("Play as O")) {
			m_VoxelClient.SetPlayerChoice(Player::O);
		}
		
		ImGui::SeparatorText("Make Move");
		
		ImGui::SliderInt("X", &m_MoveIndexes[0], 0, 2);
		ImGui::SliderInt("Y", &m_MoveIndexes[1], 0, 2);
		ImGui::SliderInt("Z", &m_MoveIndexes[2], 0, 2);
		
		if (ImGui::Button("Send Move")) {
			if (m_VoxelClient.SendMove(m_MoveIndexes[0], m_MoveIndexes[1], m_MoveIndexes[2])) {
				std::cout << "Move sent: (" << m_MoveIndexes[0] << ", " 
				          << m_MoveIndexes[1] << ", " << m_MoveIndexes[2] << ")\n";
			} else {
				std::cout << "Not your turn or unable to send move.\n";
			}
		}

		ImGui::SeparatorText("Chunk Editing");

		const std::size_t chunkCount = m_Builder.GetChunkCount();
		if (chunkCount == 0) {
			ImGui::TextUnformatted("No chunks available.");
			return;
		}

		int chunkIndex = static_cast<int>(m_SelectedChunkIndex);
		if (ImGui::SliderInt("Chunk Index", &chunkIndex, 0, static_cast<int>(chunkCount) - 1)) {
			m_SelectedChunkIndex = static_cast<std::size_t>(chunkIndex);
		}

		if (ImGui::Button("Randomize Chunk")) {
			if (Chunk* chunk = m_Builder.GetChunk(m_SelectedChunkIndex)) {
				chunk->SetChunkSomeSolid();
				RebuildAndUploadChunk(m_SelectedChunkIndex);
			}
		}

		ImGui::SameLine();
		if (ImGui::Button("Solid Chunk")) {
			if (Chunk* chunk = m_Builder.GetChunk(m_SelectedChunkIndex)) {
				chunk->SetChunkSolid();
				RebuildAndUploadChunk(m_SelectedChunkIndex);
			}
		}

		ImGui::SameLine();
		if (ImGui::Button("Clear Chunk")) {
			if (Chunk* chunk = m_Builder.GetChunk(m_SelectedChunkIndex)) {
				FillChunk(*chunk, VoxelType::Air);
				RebuildAndUploadChunk(m_SelectedChunkIndex);
			}
		}
		
		ImGui::SameLine();
		if (ImGui::Button("Sphere")) {
			if (Chunk* chunk = m_Builder.GetChunk(m_SelectedChunkIndex)) {
				chunk->SetChunkPlayer(Player::O);
				RebuildAndUploadChunk(m_SelectedChunkIndex);
			}
		}
		
		ImGui::SameLine();
		if (ImGui::Button("X")) {
			if (Chunk* chunk = m_Builder.GetChunk(m_SelectedChunkIndex)) {
				chunk->SetChunkPlayer(Player::X);
				RebuildAndUploadChunk(m_SelectedChunkIndex);
			}
		}
	}

	void TestChunk::UpdateChunksFromBoard()
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

			// Validate coordinates
			if (x < 0 || x >= 3 || y < 0 || y >= 3 || z < 0 || z >= 3) {
				continue;
			}

			// Calculate chunk index
			std::size_t chunkIndex = x + y * gridSize.x + z * gridSize.x * gridSize.y;
			
			Chunk* chunk = m_Builder.GetChunk(chunkIndex);
			if (chunk) {
				// Update chunk with player from board
				Player cellPlayer = gameState.board[x][y][z];
				chunk->SetChunkPlayer(cellPlayer);
				
				// Rebuild and upload the mesh
				RebuildAndUploadChunk(chunkIndex);
			}
		}

		// Clear the updated chunks list
		m_VoxelClient.ClearUpdatedChunks();
	}

	void TestChunk::RebuildAndUploadChunk(std::size_t index)
	{
		if (!m_Builder.RebuildChunkMesh(index)) {
			return;
		}

		if (const Mesh* mesh = m_Builder.GetChunkMesh(index)) {
			m_Renderer.UpdateChunkMesh(index, *mesh);
		}
	}

	void TestChunk::FillChunk(Chunk& chunk, VoxelType type)
	{
		for (int x = 0; x < chunk.GetSizeX(); ++x) {
			for (int y = 0; y < chunk.GetSizeY(); ++y) {
				for (int z = 0; z < chunk.GetSizeZ(); ++z) {
					chunk.SetIndex(x, y, z, type);
				}
			}
		}
	}
}