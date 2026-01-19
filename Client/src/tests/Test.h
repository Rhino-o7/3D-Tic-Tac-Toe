#pragma once
#include <vector>
#include <string>
#include <functional>
namespace Test {

	class Test {
	private:

	public:
		Test() {}
		virtual ~Test() {}
		
		virtual void OnUpdate(float deltaTime) {}
		virtual void OnRender() {}
		virtual void OnImGuiRender() {}
	};

	class TestMenu : public Test {
	private:
		std::vector<std::pair<std::string, std::function<Test*()>>> m_Tests;
		Test*& m_CurrentTest;
	public:
		TestMenu(Test*& currentTestPtr);

		void OnImGuiRender() override;

		template<typename T>
		void RegisterTest(const std::string& name)
		{
			m_Tests.push_back(std::make_pair(name, []() { return new T(); }));
		}
	};
	
}