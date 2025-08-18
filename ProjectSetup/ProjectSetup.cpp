#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>

struct ProjectProperties
{
	std::string Name;
	bool CreateLexer = false;
	std::string LexerName;
	bool CreateParser = false;
	std::string ParserName;
};

static bool YesOrNoChoice(const std::string& message)
{
	std::string choice;
	do
	{
		std::cout << message;
		std::getline(std::cin, choice);
		std::cout << std::endl;
	} while (choice.size() == 1 && std::tolower(choice.back()) == 'y' && std::tolower(choice.back()) == 'n');
	if (choice.back() == 'y')
		return true;
	return false;
}

static bool ReplacePlaceholders(std::string& sourceString, const std::string& placeHolder, const std::string& replaceString)
{
	size_t pos = 0;
	while ((pos = sourceString.find(placeHolder, pos)) != std::string::npos)
	{
		sourceString.replace(pos, placeHolder.length(), replaceString);
		pos += replaceString.length();
	}
	return true;
}

int main(int argc, char** argv)
{
	if (argc < 9)
	{
		std::cout << "Invalid argument count" << std::endl;
		exit(1);
	}
	const std::string mainPremakePath = argv[1];
	const std::string premakeTemplatePath = argv[2];
	const std::string mainFilePath = argv[3];
	const std::string tokenAndRulesIDFilePath = argv[4];
	const std::string lexerTemplateHeaderFilePath = argv[5];
	const std::string lexerTemplateSourceFilePath = argv[6];
	const std::string parserTemplateHeaderFilePath = argv[7];
	const std::string parserTemplateSourceFilePath = argv[8];

	ProjectProperties projectProps;
	do
	{
		std::cout << "Enter project name: ";
		std::getline(std::cin, projectProps.Name);
		std::cout << std::endl;
	} while (!YesOrNoChoice("Confirm [y/n]: "));

	if (projectProps.CreateLexer = YesOrNoChoice("Do you want to add lexer template [y/n]: "))
	{
		do
		{
			std::cout << "Enter lexer name: ";
			std::getline(std::cin, projectProps.LexerName);
			std::cout << std::endl;
		} while (!YesOrNoChoice("Confirm [y/n]: "));
	}

	if (projectProps.CreateLexer && (projectProps.CreateParser = YesOrNoChoice("Do you want to add parser template [y/n]: ")))
	{
		do
		{
			std::cout << "Enter parser name: ";
			std::getline(std::cin, projectProps.ParserName);
			std::cout << std::endl;
		} while (!YesOrNoChoice("Confirm [y/n]: "));
	}

	/////PREMAKE//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::ofstream mainPremakeStream(mainPremakePath, std::ios::app);

	std::ifstream premakeTemplateStream(premakeTemplatePath);
	std::stringstream premakeTemplateSS;
	premakeTemplateSS << premakeTemplateStream.rdbuf();
	std::string premakeTemplateString = premakeTemplateSS.str();
	ReplacePlaceholders(premakeTemplateString, "#projectName#", projectProps.Name);
	mainPremakeStream << std::endl << std::endl << premakeTemplateString;
	premakeTemplateStream.close();
	mainPremakeStream.close();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::filesystem::path premakePath = mainPremakePath;
	std::filesystem::path parentDir = premakePath.parent_path();

	std::filesystem::path projectDir = parentDir / projectProps.Name;
	std::filesystem::path srcDir = projectDir / "src";

	if (!std::filesystem::exists(projectDir)) 
	{
        std::filesystem::create_directories(srcDir);
        std::cout << "Created project folder: " << projectDir << std::endl;

		{
			std::filesystem::path destFile = srcDir / std::filesystem::path(mainFilePath).filename();
			std::filesystem::copy_file(mainFilePath, destFile, std::filesystem::copy_options::overwrite_existing);
			std::cout << "Created main.cpp"<< std::endl;
		}
		
		{
			std::filesystem::path destFile = srcDir / std::filesystem::path(tokenAndRulesIDFilePath).filename();
			std::filesystem::copy_file(tokenAndRulesIDFilePath, destFile, std::filesystem::copy_options::overwrite_existing);
			std::cout << "Created RuleAndTokenIDS.h"<< std::endl;
		}

		if(projectProps.CreateLexer)
		{
			std::filesystem::path lexerHeader = srcDir / (projectProps.LexerName + ".h");
			std::filesystem::path lexerSource = srcDir / (projectProps.LexerName + ".cpp");
			if (!std::filesystem::exists(lexerHeader)) 
			{
				std::ifstream in(lexerTemplateHeaderFilePath);
				std::stringstream inSS;
				inSS << in.rdbuf();
				std::string inStr = inSS.str();
				std::ofstream out(lexerHeader);
				ReplacePlaceholders(inStr, "#lexerName#", projectProps.LexerName);
				out << inStr;
				out.close();
				std::cout << "Created " << projectProps.LexerName << ".h" << std::endl;
			}
			if (!std::filesystem::exists(lexerSource)) 
			{
				std::ifstream in(lexerTemplateSourceFilePath);
				std::stringstream inSS;
				inSS << in.rdbuf();
				std::string inStr = inSS.str();
				std::ofstream out(lexerSource);
				ReplacePlaceholders(inStr, "#lexerName#", projectProps.LexerName);
				out << inStr;
				out.close();
				std::cout << "Created " << projectProps.LexerName << ".cpp" << std::endl;
			}

			if(projectProps.CreateParser)
			{
				std::filesystem::path parserHeader = srcDir / (projectProps.ParserName + ".h");
				std::filesystem::path parserSource = srcDir / (projectProps.ParserName + ".cpp");
				if (!std::filesystem::exists(parserHeader)) 
				{
					std::ifstream in(parserTemplateHeaderFilePath);
					std::stringstream inSS;
					inSS << in.rdbuf();
					std::string inStr = inSS.str();
					std::ofstream out(parserHeader);
					ReplacePlaceholders(inStr, "#lexerName#", projectProps.LexerName);
					ReplacePlaceholders(inStr, "#parserName#", projectProps.ParserName);
					out << inStr;
					out.close();
					std::cout << "Created " << projectProps.ParserName << ".h" << std::endl;
				}
				if (!std::filesystem::exists(parserSource)) 
				{
					std::ifstream in(parserTemplateSourceFilePath);
					std::stringstream inSS;
					inSS << in.rdbuf();
					std::string inStr = inSS.str();
					std::ofstream out(parserSource);
					ReplacePlaceholders(inStr, "#lexerName#", projectProps.LexerName);
					ReplacePlaceholders(inStr, "#parserName#", projectProps.ParserName);
					out << inStr;
					out.close();
					std::cout << "Created " << projectProps.ParserName << ".cpp" << std::endl;
				}
			}
		}
    } 
	else 
	{
        std::cout << "Project folder already exists: " << projectDir << "\n";
    }

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}