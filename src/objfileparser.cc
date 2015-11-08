#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <iostream>

#include "mtlfileparser.hh"
#include "objparser.hh"

using namespace ObjParser;

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
 * <material library> -> mtllib FILENAME
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
    START,
    FLOAT_LITERAL,
    INT_LITERAL,
    FILENAME,
    GEOMETRIC_VERTEX_INDICATOR,
    VERTEX_TEXTURE_INDICATOR,
    VERTEX_NORMAL_INDICATOR,
    OBJECT_NAME_INDICATOR,
    POLYGON_FACE_INDICATOR,
    MATERIAL_LIBRARY_INDICATOR,
    GROUP_INDICATOR,
    SMOOTHING_GROUP,
    MATERIAL_NAME,
    IDENTIFIER,
    NEWLINE,
    INDEX_SEPARATOR,
    SCANEOF
};

std::string GetStringForToken(Token token)
{
    switch (token)
    {
    case Token::START:
        return "START";
    case Token::FLOAT_LITERAL:
        return "FLOAT_LITERAL";
    case Token::INT_LITERAL:
        return "INT_LITERAL";
    case Token::FILENAME:
        return "FILENAME";
    case Token::GEOMETRIC_VERTEX_INDICATOR:
        return "GEOMETRIC_VERTEX_INDICATOR";
    case Token::VERTEX_TEXTURE_INDICATOR:
        return "VERTEX_TEXTURE_INDICATOR";
    case Token::VERTEX_NORMAL_INDICATOR:
        return "VERTEX_NORMAL_INDICATOR";
    case Token::GROUP_INDICATOR:
        return "GROUP_INDICATOR";
    case Token::SMOOTHING_GROUP:
        return "SMOOTHING_GROUP";
    case Token::MATERIAL_NAME:
        return "MATERIAL_NAME";
    case Token::MATERIAL_LIBRARY_INDICATOR:
        return "MATERIAL_LIBRARY_INDICATOR";
    case Token::OBJECT_NAME_INDICATOR:
        return "OBJECT_NAME_INDICATOR";
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
        : _fileName(fileName), _currentToken(Token::START), _line(1)
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
            return;
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
        
        bool isLineStart = _currentToken == Token::NEWLINE || _currentToken == Token::START;
        if (isLineStart)
        {
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

            if (input == 'm')
            {
                _fileStream.unget();
                if (checkForString("mtllib"))
                {
                    _currentToken = Token::MATERIAL_LIBRARY_INDICATOR;
                    return;
                }
                _fileStream.get();
            }

            if (input == 'o' && nextInput == ' ')
            {
                _currentToken = Token::OBJECT_NAME_INDICATOR;
                return;
            }
            
            if (input == 's' && nextInput == ' ')
            {
                _currentToken = Token::SMOOTHING_GROUP;
                return;
            }
        }
        else // !isLineStart
        {
            if (input == '/')
            {
                _currentToken = Token::INDEX_SEPARATOR;
                return;
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

            if (isalnum(input))
            {
                if (_currentToken == Token::MATERIAL_LIBRARY_INDICATOR)
                {
                    _currentToken = Token::FILENAME;

                    while (isalnum(input) || input == '-' || input == '_' || input == ':' || input == '.')
                    {
                        _tokenBuffer.push_back(input);
                        input = _fileStream.get();
                    }
                }
                else
                {
                    _currentToken = Token::IDENTIFIER;
                    while (isalnum(input) || input == '-' || input == '_' || input == '.')
                    {
                        _tokenBuffer.push_back(input);
                        input = _fileStream.get();
                    }
                }

                _fileStream.unget();
                return;
            }
        }
        
        std::stringstream errorStream;
        errorStream << "Obj scanning error: file: " << _fileName << " line: " << _line << std::endl;
        errorStream << " char: " << input << std::endl;
        throw std::runtime_error(errorStream.str());
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
};

class ObjParseResult : public IParseResult
{
public:
    std::vector<Vertex> GetVertices() const
    {
        return _vertices;
    }

    std::vector<Normal> GetNormals() const
    {
        return _normals;
    }

    std::vector<UVCoord> GetUVCoords() const
    {
        return _uvCoords;
    }

    std::vector<IndexValue> GetIndices() const
    {
        return _indices;
    }

    std::vector<Face> GetFaces() const
    {
        return _faces;
    }

    std::vector<Material*> GetMaterials() const
    {
        return _materials;
    }

    std::vector<Vertex> _vertices;
    std::vector<Normal> _normals;
    std::vector<UVCoord> _uvCoords;
    std::vector<IndexValue> _indices;
    std::vector<Face> _faces;
    std::vector<Material*> _materials;
};

class ObjFileParserImplementation : public ObjFileParser::IObjFileParserImplementation
{
public:
    ObjFileParserImplementation(const char *path, const char *fileName)
        : _path(path), _fileName(fileName), _scanner(NULL), _currentMaterial(NULL)
    {
    }

    ~ObjFileParserImplementation()
    {
        if (_scanner != NULL)
            delete _scanner;
    }

    IParseResult *Parse();

    vector<float> GetVertices();
    vector<float> GetNormals();
    vector<IndexValue> GetIndices();
    vector<Face> GetFaces();
    
private:
    const char *_path;
    const char *_fileName;
    ObjTokenScanner *_scanner;
    ObjParseResult *_result;
    Material *_currentMaterial;
    void MatchValue();
    void MatchLine();
};

ObjFileParser::ObjFileParser(const char *path, const char *filename)
    : _implementation(new ObjFileParserImplementation(path, filename))
{
}

ObjFileParser::~ObjFileParser()
{
    delete _implementation;
}

IParseResult *ObjFileParser::Parse()
{
    return _implementation->Parse();
}

IParseResult *ObjFileParserImplementation::Parse()
{
    _scanner = new ObjTokenScanner((string(_path) + _fileName).c_str());
    _result = new ObjParseResult();

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

    return _result;
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
        errorStream << "Obj parse error: Expected value type: " << _fileName << " line: " << _scanner->GetLine();
        throw std::runtime_error(errorStream.str());
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

        _result->_vertices.push_back(vertex);
    }
    else if (_scanner->GetCurrentToken() == Token::VERTEX_NORMAL_INDICATOR)
    {
        Normal normal;
        _scanner->MatchToken(Token::VERTEX_NORMAL_INDICATOR);
        normal.coordinates[0] = atof(_scanner->GetTokenBuffer().c_str());        
        MatchValue();
        normal.coordinates[1] = atof(_scanner->GetTokenBuffer().c_str());        
        MatchValue();
        normal.coordinates[2] = atof(_scanner->GetTokenBuffer().c_str());        
        MatchValue();

        _result->_normals.push_back(normal);
    }
    else if (_scanner->GetCurrentToken() == Token::VERTEX_TEXTURE_INDICATOR)
    {
        UVCoord uvCoord;
        _scanner->MatchToken(Token::VERTEX_TEXTURE_INDICATOR);
        uvCoord.coordinates[0] = atof(_scanner->GetTokenBuffer().c_str());
        MatchValue();
        uvCoord.coordinates[1] = atof(_scanner->GetTokenBuffer().c_str());
        MatchValue();

        _result->_uvCoords.push_back(uvCoord);
    }
    else if (_scanner->GetCurrentToken() == Token::GROUP_INDICATOR)
    {
        _scanner->MatchToken(Token::GROUP_INDICATOR);

        while (_scanner->GetCurrentToken() == Token::IDENTIFIER)
        {
            _scanner->MatchToken(Token::IDENTIFIER);
        }
    }
    else if (_scanner->GetCurrentToken() == Token::SMOOTHING_GROUP)
    {
        _scanner->MatchToken(Token::SMOOTHING_GROUP);

        if (_scanner->GetCurrentToken() == Token::IDENTIFIER)
        {
            if (_scanner->GetTokenBuffer() == "off")
            {
                _scanner->MatchToken(Token::IDENTIFIER);
            }
            else
            {
                std::stringstream errorStream;
                errorStream << "Obj parse error: Expected \"off\": " << _fileName << " line: " << _scanner->GetLine();
                throw std::runtime_error(errorStream.str());
            }
        }
    }
    else if (_scanner->GetCurrentToken() == Token::OBJECT_NAME_INDICATOR)
    {
        _scanner->MatchToken(Token::OBJECT_NAME_INDICATOR);

        while (_scanner->GetCurrentToken() == Token::IDENTIFIER)
        {
            _scanner->MatchToken(Token::IDENTIFIER);
        }
    }
    else if (_scanner->GetCurrentToken() == Token::POLYGON_FACE_INDICATOR)
    {
        Face face;
        _scanner->MatchToken(Token::POLYGON_FACE_INDICATOR);
        face.material = _currentMaterial;

        while (_scanner->GetCurrentToken() != Token::NEWLINE)
        {
            face.vertexIndices.push_back(atoi(_scanner->GetTokenBuffer().c_str()) - 1);
            _scanner->MatchToken(Token::INT_LITERAL);

            if (_scanner->GetCurrentToken() == Token::INDEX_SEPARATOR)
            {
                _scanner->MatchToken(Token::INDEX_SEPARATOR);

                if (_scanner->GetCurrentToken() == Token::INT_LITERAL)
                {
                    face.UVIndices.push_back(atoi(_scanner->GetTokenBuffer().c_str()) - 1);
                    _scanner->MatchToken(Token::INT_LITERAL);
                }

                if (_scanner->GetCurrentToken() == Token::INDEX_SEPARATOR)
                {
                    _scanner->MatchToken(Token::INDEX_SEPARATOR);
                    face.normalIndices.push_back(atoi(_scanner->GetTokenBuffer().c_str()) - 1);
                    _scanner->MatchToken(Token::INT_LITERAL);
                }
            }
        }

        _result->_faces.push_back(face);
    }
    else if (_scanner->GetCurrentToken() == Token::MATERIAL_NAME)
    {
        _scanner->MatchToken(Token::MATERIAL_NAME);
        string name = _scanner->GetTokenBuffer();
        _scanner->MatchToken(Token::IDENTIFIER);

        _currentMaterial = NULL;
        for (int i = 0; i < _result->_materials.size(); ++i)
        {
            if (_result->_materials[i]->name == name)
            {
                _currentMaterial = _result->_materials[i];
                break;
            }
        }

        if (_currentMaterial == NULL)
        {
            std::stringstream errorStream;
            errorStream << "Parse error: coult not find referenced material: " << name << " - " << _fileName << " line: " << _scanner->GetLine();
            throw std::runtime_error(errorStream.str());
        }
    }
    else if (_scanner->GetCurrentToken() == Token::MATERIAL_LIBRARY_INDICATOR)
    {
        _scanner->MatchToken(Token::MATERIAL_LIBRARY_INDICATOR);
        string fileName = _scanner->GetTokenBuffer();
        _scanner->MatchToken(Token::FILENAME);            

        MtlFileParser mtlParser(_path, fileName.c_str());
        mtlParser.Parse();

        vector<Material*> newMaterials = mtlParser.GetMaterials();
        _result->_materials.reserve(_result->_materials.size() + newMaterials.size());
        _result->_materials.insert(_result->_materials.end(), newMaterials.begin(), newMaterials.end());
    }

    _scanner->MatchToken(Token::NEWLINE);
}
