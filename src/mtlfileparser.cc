#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "mtlfileparser.hh"

using namespace ObjParser;

using std::ifstream;
using std::string;
using std::unordered_map;
using std::vector;

/*
 * Grammar:
 * <ambient color> -> Ka FLOAT FLOAT FLOAT
 * <material> -> newmtl IDENTIFIER
 * <line> -> <line definition> LINEEND
 * <line definition> -> <material> | <ambient color>
 */

class MtlFileParserImplementation : public MtlFileParser::IMtlFileParserImplementation
{
private:
    enum class Token
    {
        Start,
        Float,
        AmbientColor,
        DiffuseColor,
        DiffuseMap,
        DissolveFactor,
        IlluminationModel,
        OpticalDensity,
        SpecularColor,
        SpecularExponent,
        LineEnd,
        FileName,
        Identifier,
        Integer,
        MaterialName,
        ScanEOF
    };

    static string GetStringForToken(Token token)
    {
        switch (token)
        {
        case Token::Start:
            return "Start";
        case Token::Float:
            return "Float";
        case Token::AmbientColor:
            return "AmbientColor";
        case Token::DiffuseColor:
            return "DiffuseColor";
        case Token::DiffuseMap:
            return "DiffuseMap";
        case Token::DissolveFactor:
            return "DissolveFactor";
        case Token::IlluminationModel:
            return "IlluminationModel";
        case Token::OpticalDensity:
            return "OpticalDensity";
        case Token::SpecularColor:
            return "SpecularColor";
        case Token::SpecularExponent:
            return "SpecularExponent";
        case Token::LineEnd:
            return "LineEnd";
        case Token::FileName:
            return "FileName";
        case Token::Identifier:
            return "Identifier";
        case Token::Integer:
            return "Integer";
        case Token::MaterialName:
            return "MaterialName";
        case Token::ScanEOF:
            return "ScanEOF";
        }
    }

    class MtlTokenScanner
    {
    public:
        MtlTokenScanner(const char *fileName)
            : _fileName(fileName), _currentToken(Token::Start), _line(1)
        {
            _fileStream.open(fileName);
        }

        ~MtlTokenScanner()
        {
            _fileStream.close();
        }

        int GetLine() const
        {
            return _line;
        }

        bool isOpen() const
        {
            return _fileStream.is_open();
        }

        Token GetCurrentToken() const
        {
            return _currentToken;
        }

        string GetTokenBuffer() const
        {
            return _tokenBuffer;
        }

        void MatchToken(Token token)
        {
            if (_currentToken == token)
            {
                GetNextToken();
            }
            else
            {
                std::stringstream errorStream;
                errorStream << "Mtl syntax error: file: " << _fileName << " line: " << _line << std::endl;
                errorStream << "   Expected: " << GetStringForToken(token) << " Actual: " << GetStringForToken(_currentToken) << std::endl;
                if (_currentToken == Token::Identifier)
                {
                    errorStream << "   Identifier: " << _tokenBuffer << std::endl;
                }
                throw std::runtime_error(errorStream.str());
            }
        }
    
    private:
        string _fileName;
        ifstream _fileStream;
        Token _currentToken;
        unsigned int _line;
        string _tokenBuffer;

        bool checkForString(const std::string s)
        {
            string actual;
        
            for (actual = ""; actual.size() <= s.size(); actual.push_back(_fileStream.get()))
            {
                if (actual != s.substr(0, actual.size()))
                {
                    for (int i = 0; i < actual.size(); ++i)
                        _fileStream.unget();
                    return false;
                }
            }

            return true;
        }

        void GetNextToken()
        {
            char input = _fileStream.get();
            _tokenBuffer = "";

            while (input == ' ' || input == '#')
            {
                if (input == '#')
                {
                    for (input = _fileStream.get(); input != '\n' && input != EOF; input = _fileStream.get())  {}
                    if (input == '\n')
                        ++_line;
                }
                input = _fileStream.get();
            }

            if (input == EOF)
            {
                _currentToken = Token::ScanEOF;
                return;
            }
            
            if (input == '\n')
            {
                _currentToken = Token::LineEnd;
                ++_line;
                return;
            }

            bool isLineStart = _currentToken == Token::LineEnd || _currentToken == Token::Start;
            if (isLineStart)
            {
                _fileStream.unget();
                
                if (checkForString("newmtl"))
                {
                    _currentToken = Token::MaterialName;
                    return;
                }

                if (checkForString("Ka"))
                {
                    _currentToken = Token::AmbientColor;
                    return;
                }

                if (checkForString("Kd"))
                {
                    _currentToken = Token::DiffuseColor;
                    return;
                }

                if (checkForString("map_Kd"))
                {
                    _currentToken = Token::DiffuseMap;
                    return;
                }

                if (checkForString("Ks"))
                {
                    _currentToken = Token::SpecularColor;
                    return;
                }

                if (checkForString("Ns"))
                {
                    _currentToken = Token::SpecularExponent;
                    return;
                }

                if (checkForString("Ni"))
                {
                    _currentToken = Token::OpticalDensity;
                    return;
                }

                if (checkForString("d"))
                {
                    _currentToken = Token::DissolveFactor;
                    return;
                }

                if (checkForString("illum"))
                {
                    _currentToken = Token::IlluminationModel;
                    return;
                }
            }
            else
            {
                if (input == '-' || input == '.' || isdigit(input)) // float or int
                {
                    _currentToken = Token::Integer;
                    while (input == '-' || input == '.' || isdigit(input))
                    {
                        _tokenBuffer.push_back(input);
                        if (input == '.')
                            _currentToken = Token::Float;
                        input = _fileStream.get();
                    }

                    _fileStream.unget();
                    return;
                }

                if (isalnum(input))
                {
                    _currentToken = Token::Identifier;
                    while (isalnum(input) || input == '.' || input == '-' || input == '_')
                    {
                        _tokenBuffer.push_back(input);
                        input = _fileStream.get();
                    }

                    if (input == '\\' || input == ':')
                    {
                        _currentToken = Token::FileName;

                        do 
                        {
                            _tokenBuffer.push_back(input);
                            input = _fileStream.get();
                        } while (isalnum(input) || input == '.' || input == '-' || input == '_' || input == '\\');
                    }

                    _fileStream.unget();
                    return;
                }
            }
        
            std::stringstream errorStream;
            errorStream << "Scanning error: file: " << _fileName << " line: " << _line << std::endl;
            throw std::runtime_error(errorStream.str());
        }
    };
    
public:
    MtlFileParserImplementation(const char *path, const char *fileName)
        : _path(path), _fileName(fileName), _scanner(NULL), _currentMaterial(NULL)
    {
    }

    void Parse()
    {
        _scanner = new MtlTokenScanner((string(_path) + _fileName).c_str());

        if (!_scanner->isOpen())
        {
            std::stringstream errorStream;
            errorStream << "Runtime error: unable to open MTL file: " << _fileName;
            throw std::runtime_error(errorStream.str());
        }

        _scanner->MatchToken(Token::Start);
        
        while (_scanner->GetCurrentToken() != Token::ScanEOF)
        {
            MatchLine();
        }

        delete _scanner;
    }

    vector<Material*> GetMaterials() const
    {
        return _materials;
    }
    
private:
    const char *_path;
    const char *_fileName;
    MtlTokenScanner *_scanner;
    Material *_currentMaterial;
    vector<Material*> _materials;

    void MatchLine()
    {
        if (_scanner->GetCurrentToken() == Token::MaterialName)
        {
            _scanner->MatchToken(Token::MaterialName);
            _currentMaterial = new Material();
            _currentMaterial->name = _scanner->GetTokenBuffer();
            _materials.push_back(_currentMaterial);
            _scanner->MatchToken(Token::Identifier);
        }
        else if (_scanner->GetCurrentToken() == Token::AmbientColor)
        {
            _scanner->MatchToken(Token::AmbientColor);
            ColorValue color;
            color.red = atof(_scanner->GetTokenBuffer().c_str());
            MatchValue();
            color.green = atof(_scanner->GetTokenBuffer().c_str());
            MatchValue();
            color.blue = atof(_scanner->GetTokenBuffer().c_str());
            MatchValue();

            _currentMaterial->ambientColor = color;
        }
        else if (_scanner->GetCurrentToken() == Token::DiffuseColor)
        {
            _scanner->MatchToken(Token::DiffuseColor);
            ColorValue color;
            color.red = atof(_scanner->GetTokenBuffer().c_str());
            MatchValue();
            color.green = atof(_scanner->GetTokenBuffer().c_str());
            MatchValue();
            color.blue = atof(_scanner->GetTokenBuffer().c_str());
            MatchValue();

            _currentMaterial->diffuseColor = color;
        }
        else if (_scanner->GetCurrentToken() == Token::SpecularColor)
        {
            _scanner->MatchToken(Token::SpecularColor);
            ColorValue color;
            color.red = atof(_scanner->GetTokenBuffer().c_str());
            MatchValue();
            color.green = atof(_scanner->GetTokenBuffer().c_str());
            MatchValue();
            color.blue = atof(_scanner->GetTokenBuffer().c_str());
            MatchValue();

            _currentMaterial->specularColor = color;
        }
        else if (_scanner->GetCurrentToken() == Token::OpticalDensity)
        {
            _scanner->MatchToken(Token::OpticalDensity);
            MatchValue();
        }
        else if (_scanner->GetCurrentToken() == Token::DissolveFactor)
        {
            _scanner->MatchToken(Token::DissolveFactor);
            MatchValue();
        }
        else if (_scanner->GetCurrentToken() == Token::IlluminationModel)
        {
            _scanner->MatchToken(Token::IlluminationModel);
            _scanner->MatchToken(Token::Integer);
        }
        else if (_scanner->GetCurrentToken() == Token::DiffuseMap)
        {
            _scanner->MatchToken(Token::DiffuseMap);
            _currentMaterial->diffuseMap = _scanner->GetTokenBuffer();
            _scanner->MatchToken(Token::FileName);
        }
        else if (_scanner->GetCurrentToken() == Token::SpecularExponent)
        {
            _scanner->MatchToken(Token::SpecularExponent);
            _currentMaterial->specularExponent = atof(_scanner->GetTokenBuffer().c_str());
            MatchValue();
        }

        _scanner->MatchToken(Token::LineEnd);
    }

    void MatchValue()
    {
        if (_scanner->GetCurrentToken() == Token::Float)
        {
            _scanner->MatchToken(Token::Float);
        }
        else if (_scanner->GetCurrentToken() == Token::Integer)
        {
            _scanner->MatchToken(Token::Integer);
        }
        else
        {
            std::stringstream errorStream;
            errorStream << "Mtl parse error: Expected value type: " << _fileName << " line: " << _scanner->GetLine();
            throw std::runtime_error(errorStream.str());
        }
    }
};

MtlFileParser::MtlFileParser(const char *path, const char *fileName)
    : _implementation(new MtlFileParserImplementation(path, fileName))
{
}

MtlFileParser::~MtlFileParser()
{
    delete _implementation;
}

void MtlFileParser::Parse()
{
    _implementation->Parse();
}

vector<Material*> MtlFileParser::GetMaterials() const
{
    return _implementation->GetMaterials();
}
