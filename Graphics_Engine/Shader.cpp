#include <fstream>
#include <iostream>
#include "Shader.h"



Shader* Shader::Instance()
{
    static Shader* shader = new Shader();
    return shader;
}

Shader::Shader()
{
    m_shaderProgramID = 0;
    m_vertexShaderID = 0;
    m_fragmentShaderID = 0;
}

bool Shader::CreateProgram()
{
    m_shaderProgramID = glCreateProgram();

    if (m_shaderProgramID == 0) {
        std::cout << "Error creating shader program" << std::endl;
        return false;
    }
    return true;
}

bool Shader::CreateShaders()
{
    m_vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    if (m_vertexShaderID == 0) {
        std::cout << "Error creating vertex shader object" << std::endl;
        return false;
    }

    m_fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    if (m_fragmentShaderID == 0) {
        std::cout << "Error creating fragment shader object" << std::endl;
        return false;
    }

    return true;
}

bool Shader::CompileShaders(const std::string& filename, ShaderType shaderType)
{
    GLuint shaderID = (shaderType == ShaderType::VERTEX_SHADER) ? m_vertexShaderID : m_fragmentShaderID;
    std::fstream file;
    std::string shaderCode;
    std::string line;

    file.open(filename);

    if (!file) {
        std::cout << "Error reading shader file: " << filename << std::endl;
        return false;
    }

    while (!file.eof())
    {
        std::getline(file, line);
        shaderCode += line + "\n";
    }
    file.close();

    // OpenGL reads shader code as a "c string" which is a char array
    const GLchar* src = reinterpret_cast<const GLchar*>(shaderCode.c_str());

    // pass in the shader ID (either vertex or fragment), the amount of shaders, the pointer to the source code, and NULL(only relevant for multiple shaders at the same time)
    glShaderSource(shaderID, 1, &src, nullptr);

    glCompileShader(shaderID);

    GLint errorCode;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &errorCode);

    if (errorCode == GL_TRUE) {
        if (shaderType == ShaderType::VERTEX_SHADER) {
            std::cout << "Vertex Shader compiled successfully" << std::endl;
        }
        else {
            std::cout << "Fragment Shader compiled successfully" << std::endl;
        }
    }
    else {
        GLsizei bufferSize = 1000;
        GLchar errorMessage[1000];

        glGetShaderInfoLog(shaderID, bufferSize, &bufferSize, errorMessage);
        std::cout << errorMessage << std::endl;
        return false;
    }

    return true;
}

void Shader::AttachShaders()
{
    glAttachShader(m_shaderProgramID, m_vertexShaderID);
    glAttachShader(m_shaderProgramID, m_fragmentShaderID);

}

bool Shader::LinkProgram()
{
    glLinkProgram(m_shaderProgramID);
    glUseProgram(m_shaderProgramID);

    GLint errorCode;
    glGetProgramiv(m_shaderProgramID, GL_LINK_STATUS, &errorCode);

    if (errorCode == GL_TRUE) {
        std::cout << "Program linked successfully" << std::endl;
    }
    else {
        GLsizei bufferSize = 1000;
        GLchar errorMessage[1000];

        glGetProgramInfoLog(m_shaderProgramID, bufferSize, &bufferSize, errorMessage);
        std::cout << errorMessage << std::endl;
        return false;
    }

    return true;
}

void Shader::DetachShaders()
{
    glDetachShader(m_shaderProgramID, m_vertexShaderID);
    glDetachShader(m_shaderProgramID, m_fragmentShaderID);

}

void Shader::DestroyShaders()
{
    glDeleteShader(m_vertexShaderID);
    glDeleteShader(m_fragmentShaderID);

}

void Shader::DestroyProgram()
{
    glDeleteProgram(m_shaderProgramID);
}

GLint Shader::GetUniformID(const std::string& uniformName) {

    GLint id = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());
    if (id == -1) {
        std::cout << "Variable " << uniformName << " is not defined or not used." << std::endl;
        return -1;
    }
    return id;

}

bool Shader::SendUniformData(const std::string& uniformName, GLint data)
{
    GLint id = GetUniformID(uniformName);

    if (id != -1) {
        return false;
    }

    glUniform1i(id, data);
    return true;
}

bool Shader::SendUniformData(const std::string& uniformName, GLfloat data)
{
    GLint id = GetUniformID(uniformName);

    if (id == -1) {
        return false;
    }

    glUniform1f(id, data);
    return true;
}

bool Shader::SendUniformData(const std::string& uniformName, GLuint data)
{
    GLint id = GetUniformID(uniformName);

    if (id == -1) {
        return false;
    }

    glUniform1ui(id, data);
    return true;
}

bool Shader::SendUniformData(const std::string& uniformName, GLfloat x, GLfloat y)
{
    GLint id = GetUniformID(uniformName);

    if (id == -1) {
        return false;
    }

    glUniform2f(id, x, y);
    return true;
}

bool Shader::SendUniformData(const std::string& uniformName, GLfloat x, GLfloat y, GLfloat z)
{
    GLint id = GetUniformID(uniformName);

    if (id == -1) {
        return false;
    }

    glUniform3f(id, x, y, z);
    return true;
}

bool Shader::SendUniformData(const std::string& uniformName, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    GLint id = GetUniformID(uniformName);

    if (id == -1) {
        return false;
    }

    glUniform4f(id, x, y, z, w);
    return true;
}

bool Shader::SendUniformData(const std::string& uniformName, const glm::mat4& data)
{
    GLint id = GetUniformID(uniformName);

    if (id == -1) {
        return false;
    }

    glUniformMatrix4fv(id, 1, GL_FALSE, &data[0][0]);
    return true;
}

bool Shader::SendUniformData(const std::string& uniformName, const glm::vec3 data)
{
    GLint id = GetUniformID(uniformName);

    if (id == -1) {
        return false;
    }

    glUniform3fv(id, 1, glm::value_ptr(data));
    return true;
}

GLuint Shader::GetShaderProgramID()
{
    return m_shaderProgramID;
}


