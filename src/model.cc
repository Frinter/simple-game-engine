#include "model.hh"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#include "GL/gl_core_3_3.h"
#include "model.hh"

using std::string;
using std::unordered_map;
using std::vector;

enum class OperationResult
{
    Success,
    Failure
};

class ParseResult
{
public:
    ParseResult(vector<float> positions, vector<IndexValue> indices)
        : _positions(positions), _indices(indices)
    {
    }

    ParseResult(const ParseResult &other)
        : _positions(other._positions), _indices(other._indices)
    {
    }
    
    std::vector<float> GetVertexPositions() const { return _positions; }
    std::vector<IndexValue> GetVertexIndices() const { return _indices; }

private:
    vector<float> _positions;
    vector<IndexValue> _indices;
};

class ObjFileParser
{
/*
 * Grammar:
 * <geometric vertex> -> v {<value>}
 * <vertex normal> -> vn {<value>}
 * <vertex texture> -> vt {<value>}
 * <group declaration> -> g {IDENTIFIER}
 * <face element> -> f {<face index>}
 * <material name> -> usemtl IDENTIFIER
 * <face index> -> INT_LITERAL | INT_LITERAL/INT_LITERAL | INT_LITERAL/INT_LITERAL/INT_LITERAL | INT_LITRAL//INT_LITERAL
 * <value> -> INT_LITERAL | FLOAT_LITERAL
 * <line> -> <line definition> NEWLINE
 * <line definition> -> <geometric vertex> | <vertex normal> | <vertex texture> | <face element> | nil
 * <file> -> {<line>}
 */

private:
    enum class Token
    {
        FLOAT_LITERAL,
        INT_LITERAL,
        GEOMETRIC_VERTEX_INDICATOR,
        VERTEX_TEXTURE_INDICATOR,
        VERTEX_NORMAL_INDICATOR,
        POLYGON_FACE_INDICATOR,
        GROUP_INDICATOR,
        MATERIAL_NAME,
        IDENTIFIER,
        NEWLINE,
        INDEX_SEPARATOR,
        SCANEOF
    };

    string GetStringForToken(Token token)
    {
        switch (token)
        {
        case Token::FLOAT_LITERAL:
            return "FLOAT_LITERAL";
        case Token::INT_LITERAL:
            return "INT_LITERAL";
        case Token::GEOMETRIC_VERTEX_INDICATOR:
            return "GEOMETRIC_VERTEX_INDICATOR";
        case Token::VERTEX_TEXTURE_INDICATOR:
            return "VERTEX_TEXTURE_INDICATOR";
        case Token::VERTEX_NORMAL_INDICATOR:
            return "VERTEX_NORMAL_INDICATOR";
        case Token::GROUP_INDICATOR:
            return "GROUP_INDICATOR";
        case Token::MATERIAL_NAME:
            return "MATERIAL_NAME";
        case Token::POLYGON_FACE_INDICATOR:
            return "POLYGON_FACE_INDICATOR";
        case Token::IDENTIFIER:
            return "IDENTIFIER";
        case Token::NEWLINE:
            return "NEWLINE";
        case Token::INDEX_SEPARATOR:
            return "INDEX_SEPARATOR";
        case Token::SCANEOF:
            return "SCANEOF";
        }
    }

public:
    ObjFileParser(const char *filename)
        : _filename(filename), _line(1)
    {
    }

    void Parse()
    {
        _fileStream.open(_filename);

        if (!_fileStream.is_open())
        {
            std::stringstream errorStream;
            errorStream << "Runtime error: unable to open OBJ file: " << _filename;
            throw std::runtime_error(errorStream.str());
        }

        GetNextToken();
        
        while (_currentToken != Token::SCANEOF)
        {
            MatchLine();
        }

        MatchToken(Token::SCANEOF);
        
        _fileStream.close();
    }

    ParseResult GetResult() const
    {
        return ParseResult(_vertices, _indices);
    }

private:
    const char *_filename;
    unsigned int _line;
    std::ifstream _fileStream;
    string _tokenBuffer;
    Token _currentToken;
    vector<float> _vertices;
    vector<IndexValue> _indices;
    
    void MatchValue()
    {
        if (_currentToken == Token::FLOAT_LITERAL)
            MatchToken(Token::FLOAT_LITERAL);
        else if (_currentToken == Token::INT_LITERAL)
            MatchToken(Token::INT_LITERAL);
        else
        {
            std::stringstream errorStream;
            errorStream << "Parse error: Expected value type: " << _filename << " line: " << _line;
            throw std::runtime_error(errorStream.str());
        }
    }
    
    OperationResult MatchFaceIndex()
    {
        MatchToken(Token::INT_LITERAL);

        if (_currentToken == Token::INDEX_SEPARATOR)
        {
            MatchToken(Token::INDEX_SEPARATOR);

            if (_currentToken == Token::INT_LITERAL)
            {
                MatchToken(Token::INT_LITERAL);
            }
            else if (_currentToken == Token::INDEX_SEPARATOR)
            {
                MatchToken(Token::INDEX_SEPARATOR);
                MatchToken(Token::INT_LITERAL);
            }
            else
            {
                return OperationResult::Failure;
            }
        }

        return OperationResult::Success;
    }

    void MatchLine()
    {
        if (_currentToken == Token::GEOMETRIC_VERTEX_INDICATOR)
        {
            MatchToken(Token::GEOMETRIC_VERTEX_INDICATOR);
            MatchValue();
            MatchValue();
            MatchValue();
        }
        else if (_currentToken == Token::VERTEX_TEXTURE_INDICATOR)
        {
            MatchToken(Token::VERTEX_TEXTURE_INDICATOR);
            MatchValue();
            MatchValue();
            MatchValue();
        }
        else if (_currentToken == Token::GROUP_INDICATOR)
        {
            MatchToken(Token::GROUP_INDICATOR);

            while (_currentToken == Token::IDENTIFIER)
            {
                MatchToken(Token::IDENTIFIER);
            }
        }
        else if (_currentToken == Token::POLYGON_FACE_INDICATOR)
        {
            MatchToken(Token::POLYGON_FACE_INDICATOR);
            while (_currentToken != Token::NEWLINE)
            {
                MatchFaceIndex();
            }
        }
        else if (_currentToken == Token::MATERIAL_NAME)
        {
            MatchToken(Token::MATERIAL_NAME);
            MatchToken(Token::IDENTIFIER);            
        }
        
        MatchToken(Token::NEWLINE);
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
            errorStream << "Syntax error: file: " << _filename << " line: " << _line << std::endl;
            errorStream << "   Expected: " << GetStringForToken(token) << " Actual: " << GetStringForToken(_currentToken) << std::endl;
            if (_currentToken == Token::IDENTIFIER)
            {
                errorStream << "   Identifier: " << _tokenBuffer << std::endl;
            }
            throw std::runtime_error(errorStream.str());
        }
    }

    bool checkForString(const string s)
    {
        string actual;
        
        for (actual = ""; actual.size() < s.size(); actual.push_back(_fileStream.get()))
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
        _tokenBuffer.clear();

        if (_fileStream.eof())
        {
            _currentToken = Token::SCANEOF;
        }

        char input = _fileStream.get();
        char nextInput = _fileStream.peek();

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
            _currentToken = Token::SCANEOF;
            return;
        }
            
        if (input == '\n')
        {
            _currentToken = Token::NEWLINE;
            ++_line;
            return;
        }

        if (input == '/')
        {
            _currentToken = Token::INDEX_SEPARATOR;
            return;
        }

        if (input == 'u' && nextInput == 's')
        {
            _fileStream.unget();
            if (checkForString("usemtl"))
            {
                _currentToken = Token::MATERIAL_NAME;
                return;
            }
            _fileStream.get();
        }
        
        if (input == '-' || input == '.' || isdigit(input)) // float or int
        {
            _currentToken = Token::INT_LITERAL;
            while (input == '-' || input == '.' || isdigit(input))
            {
                _tokenBuffer.push_back(input);
                if (input == '.')
                    _currentToken = Token::FLOAT_LITERAL;
                input = _fileStream.get();
            }

            _fileStream.unget();
            return;
        }

        if (input == 'g' && nextInput == ' ')
        {
            _currentToken = Token::GROUP_INDICATOR;
            return;
        }

        if (input == 'f' && nextInput == ' ')
        {
            _currentToken = Token::POLYGON_FACE_INDICATOR;
            return;
        }

        if (input == 'v' && nextInput == ' ')
        {
            _currentToken = Token::GEOMETRIC_VERTEX_INDICATOR;
            return;
        }
        
        if (input == 'v' && nextInput == 't')
        {
            _fileStream.get();
            
            if (_fileStream.peek() == ' ')
            {
                _currentToken = Token::VERTEX_TEXTURE_INDICATOR;
                return;
            }
            
            _fileStream.unget();
        }

        if (input == 'v' && nextInput == 'n')
        {
            _fileStream.get();
            
            if (_fileStream.peek() == ' ')
            {
                _currentToken = Token::VERTEX_NORMAL_INDICATOR;
                return;
            }
            
            _fileStream.unget();
        }

        if (isalnum(input))
        {
            _currentToken = Token::IDENTIFIER;
            while (isalnum(input))
            {
                _tokenBuffer.push_back(input);
                input = _fileStream.get();
            }

            _fileStream.unget();
            return;
        }
        
        std::stringstream errorStream;
        errorStream << "Lexical error: file: " << _filename << " line: " << _line << std::endl;
        throw std::runtime_error(errorStream.str());
    }
};

Model::Model(const char *filename)
{
    LoadFromFile(filename);
}

void Model::LoadFromFile(const char *filename)
{
    ObjFileParser parser(filename);
    parser.Parse();
}
