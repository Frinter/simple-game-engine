#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "objparser.hh"

using std::ifstream;
using std::string;
using std::unordered_map;
using std::vector;

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

// Terminals
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

typedef struct Vertex
{
    float coordinates[4];
} Vertex;

typedef struct Face
{
    std::vector<IndexValue> vertexIndices;
} Face;

std::string GetStringForToken(Token token)
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

class ObjTokenScanner
{
public:
    ObjTokenScanner(const char *fileName)
        : _fileName(fileName)
    {
        _fileStream.open(fileName);
    }

    ~ObjTokenScanner()
    {
        _fileStream.close();
    }

    bool isOpen() const
    {
        return _fileStream.is_open();
    }

    Token GetCurrentToken() const
    {
        return _currentToken;
    }

    int GetLine() const
    {
        return _line;
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
            errorStream << "Syntax error: file: " << _fileName << " line: " << _line << std::endl;
            errorStream << "   Expected: " << GetStringForToken(token) << " Actual: " << GetStringForToken(_currentToken) << std::endl;
            if (_currentToken == Token::IDENTIFIER)
            {
                errorStream << "   Identifier: " << _tokenBuffer << std::endl;
            }
            throw std::runtime_error(errorStream.str());
        }
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
        errorStream << "Lexical error: file: " << _fileName << " line: " << _line << std::endl;
        throw std::runtime_error(errorStream.str());
    }

private:
    const char *_fileName;
    ifstream _fileStream;
    Token _currentToken;
    unsigned int _line;
    string _tokenBuffer;

    bool checkForString(const std::string s)
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
};

class ObjFileParserImplementation : public ObjFileParser::IObjFileParserImplementation
{
public:
    ObjFileParserImplementation(const char *fileName)
        : _fileName(fileName), _scanner(NULL)
    {
    }

    ~ObjFileParserImplementation()
    {
        if (_scanner != NULL)
            delete _scanner;
    }
    
    void Parse();

    vector<float> GetVertices();
    vector<IndexValue> GetIndices();
    
private:
    const char *_fileName;
    ObjTokenScanner *_scanner;
    vector<Vertex> _vertices;
    vector<Face> _faces;

    void MatchValue();
    void MatchFaceIndex();
    void MatchLine();
};

ObjFileParser::ObjFileParser(const char *filename)
    : _implementation(new ObjFileParserImplementation(filename))
{
}

ObjFileParser::~ObjFileParser()
{
    delete _implementation;
}

void ObjFileParser::Parse()
{
    _implementation->Parse();
}

vector<float> ObjFileParser::GetVertices()
{
    return _implementation->GetVertices();
}

vector<IndexValue> ObjFileParser::GetIndices()
{
    return _implementation->GetIndices();
}

vector<float> ObjFileParserImplementation::GetVertices()
{
    vector<float> processedVertices;

    for (int i = 0; i < _vertices.size(); ++i)
    {
        Vertex vertex = _vertices[i];
        for (int j = 0; j < 4; ++j)
        {
            processedVertices.push_back(vertex.coordinates[j]);
        }
    }

    return processedVertices;
}

vector<IndexValue> ObjFileParserImplementation::GetIndices()
{
    vector<IndexValue> processedIndices;

    for (int i = 0; i < _faces.size(); ++i)
    {
        Face face = _faces[i];
        for (int j = 0; j < 3; ++j)
        {
            processedIndices.push_back(face.vertexIndices[j]);
        }
    }

    return processedIndices;
}

void ObjFileParserImplementation::Parse()
{
    _scanner = new ObjTokenScanner(_fileName);

    if (!_scanner->isOpen())
    {
        std::stringstream errorStream;
        errorStream << "Runtime error: unable to open OBJ file: " << _fileName;
        throw std::runtime_error(errorStream.str());
    }

    _scanner->GetNextToken();
        
    while (_scanner->GetCurrentToken() != Token::SCANEOF)
    {
        MatchLine();
    }

    _scanner->MatchToken(Token::SCANEOF);
}

void ObjFileParserImplementation::MatchValue()
{
    if (_scanner->GetCurrentToken() == Token::FLOAT_LITERAL)
    {
        _scanner->MatchToken(Token::FLOAT_LITERAL);
    }
    else if (_scanner->GetCurrentToken() == Token::INT_LITERAL)
    {
        _scanner->MatchToken(Token::INT_LITERAL);
    }
    else
    {
        std::stringstream errorStream;
        errorStream << "Parse error: Expected value type: " << _fileName << " line: " << _scanner->GetLine();
        throw std::runtime_error(errorStream.str());
    }
}

void ObjFileParserImplementation::MatchFaceIndex()
{
    _scanner->MatchToken(Token::INT_LITERAL);

    if (_scanner->GetCurrentToken() == Token::INDEX_SEPARATOR)
    {
        _scanner->MatchToken(Token::INDEX_SEPARATOR);

        if (_scanner->GetCurrentToken() == Token::INT_LITERAL)
        {
            _scanner->MatchToken(Token::INT_LITERAL);
        }
        else if (_scanner->GetCurrentToken() == Token::INDEX_SEPARATOR)
        {
            _scanner->MatchToken(Token::INDEX_SEPARATOR);
            _scanner->MatchToken(Token::INT_LITERAL);
        }
    }
}

void ObjFileParserImplementation::MatchLine()
{
    if (_scanner->GetCurrentToken() == Token::GEOMETRIC_VERTEX_INDICATOR)
    {
        Vertex vertex;
        _scanner->MatchToken(Token::GEOMETRIC_VERTEX_INDICATOR);
        vertex.coordinates[0] = atof(_scanner->GetTokenBuffer().c_str());
        MatchValue();
        vertex.coordinates[1] = atof(_scanner->GetTokenBuffer().c_str());
        MatchValue();
        vertex.coordinates[2] = atof(_scanner->GetTokenBuffer().c_str());
        MatchValue();
        vertex.coordinates[3] = 1.0;

        _vertices.push_back(vertex);
    }
    else if (_scanner->GetCurrentToken() == Token::VERTEX_NORMAL_INDICATOR)
    {
        _scanner->MatchToken(Token::VERTEX_NORMAL_INDICATOR);
        MatchValue();
        MatchValue();
        MatchValue();
    }
    else if (_scanner->GetCurrentToken() == Token::VERTEX_TEXTURE_INDICATOR)
    {
        _scanner->MatchToken(Token::VERTEX_TEXTURE_INDICATOR);
        MatchValue();
        MatchValue();
        MatchValue();
    }
    else if (_scanner->GetCurrentToken() == Token::GROUP_INDICATOR)
    {
        _scanner->MatchToken(Token::GROUP_INDICATOR);

        while (_scanner->GetCurrentToken() == Token::IDENTIFIER)
        {
            _scanner->MatchToken(Token::IDENTIFIER);
        }
    }
    else if (_scanner->GetCurrentToken() == Token::POLYGON_FACE_INDICATOR)
    {
        Face face;
        _scanner->MatchToken(Token::POLYGON_FACE_INDICATOR);
        
        while (_scanner->GetCurrentToken() != Token::NEWLINE)
        {
            face.vertexIndices.push_back(atoi(_scanner->GetTokenBuffer().c_str()));
            MatchFaceIndex();
        }

        _faces.push_back(face);
    }
    else if (_scanner->GetCurrentToken() == Token::MATERIAL_NAME)
    {
        _scanner->MatchToken(Token::MATERIAL_NAME);
        _scanner->MatchToken(Token::IDENTIFIER);            
    }

    _scanner->MatchToken(Token::NEWLINE);
}
