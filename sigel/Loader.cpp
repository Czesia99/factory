#include "Loader.hpp"
#include "SigelEngine.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <iostream>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <vector>
#include "Utils.hpp"
#include <filesystem>

namespace sigel
{
    static glm::mat4 aiToGlmMat4(const aiMatrix4x4& from) {
        glm::mat4 to;
        to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
        to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
        to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
        to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
        return to;
    }

    static std::string findTexturePath(aiMaterial* material,
                                       const std::vector<aiTextureType>& types,
                                       const std::string& materialName,
                                       const std::string& base_dir,
                                       const std::vector<std::string>& keywords)
    {
        aiString texPath;

        // 1. Vérification dans Assimp
        for (auto type : types) {
            if (material->GetTexture(type, 0, &texPath) == AI_SUCCESS) {
                return texPath.C_Str();
            }
        }

        // 2. Fallback dossier disque si Assimp ne trouve rien
        if (!materialName.empty() && std::filesystem::exists(base_dir))
        {
            std::string matNameLower = toLower(materialName);

            for (const auto& entry : std::filesystem::directory_iterator(base_dir))
            {
                if (!entry.is_regular_file()) continue;

                std::string fileName = entry.path().filename().string();
                std::string fileNameLower = toLower(fileName);

                // Si le fichier contient le nom du matériau
                if (fileNameLower.find(matNameLower) != std::string::npos) {
                    for (const auto& kw : keywords) {
                        if (fileNameLower.find(kw) != std::string::npos) {
                            return fileName;
                        }
                    }
                }
            }
        }

        return "";
    }

    // Helper : Charge une texture unique sur le GPU
    static uint32_t loadSingleTexture(const std::string& texPath, const aiScene* scene, const std::string& base_dir)
    {
        if (texPath.empty()) return 0;

        // Vérification texture embarquée
        const aiTexture* embeddedTex = scene->GetEmbeddedTexture(texPath.c_str());
        if (!embeddedTex && texPath[0] == '*' && scene->HasTextures()) {
            int index = std::stoi(texPath.substr(1));
            if (index >= 0 && index < static_cast<int>(scene->mNumTextures)) {
                embeddedTex = scene->mTextures[index];
            }
        }

        if (embeddedTex) {
            size_t size = (embeddedTex->mHeight == 0) ? embeddedTex->mWidth : embeddedTex->mWidth * embeddedTex->mHeight * 4;
            return SigelEngine::get().vctx.resourceManager.createTextureImageFromMemory(embeddedTex->pcData, size);
        } else {
            std::string fullPath = base_dir + texPath;
            try {
                uint32_t id = SigelEngine::get().vctx.resourceManager.createTextureImage(fullPath);
                std::cout << "  [Mat] Loaded: " << texPath << " (ID: " << id << ")" << std::endl;
                return id;
            } catch (const std::exception& e) {
                std::cerr << "  [Mat] ERROR loading texture (" << fullPath << "): " << e.what() << std::endl;
            }
        }
        return 0;
    }

    // Extraction de l'ensemble des textures PBR du matériau
    static Material loadMeshMaterial(aiMesh* ai_mesh, const aiScene* scene, const std::string& base_dir)
    {
        Material mat{0, 0, 0, 0};
        if (ai_mesh->mMaterialIndex < 0) return mat;

        aiMaterial* material = scene->mMaterials[ai_mesh->mMaterialIndex];

        aiString matName;
        std::string materialName = "";
        if (material->Get(AI_MATKEY_NAME, matName) == AI_SUCCESS) {
            materialName = matName.C_Str();
        }

        std::cout << "\n[Material Processing] " << materialName << std::endl;

        // 1. Diffuse / BaseColor
        std::string diffPath = findTexturePath(
            material,
            { aiTextureType_DIFFUSE, aiTextureType_BASE_COLOR },
            materialName, base_dir,
            { "basecolor", "diffuse", "albedo", "col" }
        );
        mat.diffuseID = loadSingleTexture(diffPath, scene, base_dir);

        // 2. Metallic
        std::string metalPath = findTexturePath(
            material,
            { aiTextureType_METALNESS, aiTextureType_SPECULAR },
            materialName, base_dir,
            { "metallic", "metal" }
        );
        mat.metallicID = loadSingleTexture(metalPath, scene, base_dir);

        // 3. Roughness
        std::string roughPath = findTexturePath(
            material,
            { aiTextureType_DIFFUSE_ROUGHNESS, static_cast<aiTextureType>(16) }, // 16 = aiTextureType_ROUGHNESS
            materialName, base_dir,
            { "roughness", "rough" }
        );
        mat.roughnessID = loadSingleTexture(roughPath, scene, base_dir);

        // 4. Normal
        std::string normPath = findTexturePath(
            material,
            { aiTextureType_NORMALS, aiTextureType_HEIGHT },
            materialName, base_dir,
            { "normal", "norm", "nrm" }
        );
        mat.normalID = loadSingleTexture(normPath, scene, base_dir);

        return mat;
    }

    static void processNode(aiNode* node, const aiScene* scene, const glm::mat4& parentTransform,
                            const std::string& base_dir, std::vector<SubMesh>& outMeshes)
    {
        glm::mat4 nodeTransform = parentTransform * aiToGlmMat4(node->mTransformation);
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(nodeTransform)));

        std::cout << "[Node] Name: \"" << node->mName.C_Str()
                  << "\" | Meshes: " << node->mNumMeshes
                  << " | Children: " << node->mNumChildren << std::endl;

        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* ai_mesh = scene->mMeshes[node->mMeshes[i]];

            std::cout << "  --- SubMesh [" << i << "] (Name: " << ai_mesh->mName.C_Str() << ") ---" << std::endl;

            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;

            vertices.reserve(ai_mesh->mNumVertices);

            for (unsigned int j = 0; j < ai_mesh->mNumVertices; j++)
            {
                Vertex vertex{};

                glm::vec4 localPos(ai_mesh->mVertices[j].x, ai_mesh->mVertices[j].y, ai_mesh->mVertices[j].z, 1.0f);
                vertex.pos = glm::vec3(nodeTransform * localPos);

                if (ai_mesh->HasNormals()) {
                    glm::vec3 localNorm(ai_mesh->mNormals[j].x, ai_mesh->mNormals[j].y, ai_mesh->mNormals[j].z);
                    vertex.normal = glm::normalize(normalMatrix * localNorm);
                } else {
                    vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }

                if (ai_mesh->HasTextureCoords(0)) {
                    vertex.texCoord = { ai_mesh->mTextureCoords[0][j].x, ai_mesh->mTextureCoords[0][j].y };
                } else {
                    vertex.texCoord = glm::vec2(0.0f, 0.0f);
                }

                // --- Tangents & Bitangents (NEW) ---
                if (ai_mesh->HasTangentsAndBitangents()) {
                    glm::vec3 localTangent(ai_mesh->mTangents[j].x, ai_mesh->mTangents[j].y, ai_mesh->mTangents[j].z);
                    glm::vec3 localBitangent(ai_mesh->mBitangents[j].x, ai_mesh->mBitangents[j].y, ai_mesh->mBitangents[j].z);

                    // Transform to world space using the normalMatrix
                    glm::vec3 t = glm::normalize(normalMatrix * localTangent);
                    glm::vec3 b = glm::normalize(normalMatrix * localBitangent);
                    glm::vec3 n = vertex.normal;

                    // Gram-Schmidt orthogonalize (Ensures T is perfectly perpendicular to N)
                    t = glm::normalize(t - n * glm::dot(n, t));

                    // Calculate Handedness / Bitangent sign (W component)
                    // If the cross product of N and T points away from B, we need to flip the bitangent in the shader
                    float w = (glm::dot(glm::cross(n, t), b) < 0.0f) ? -1.0f : 1.0f;

                    vertex.tangent = glm::vec4(t, w);
                } else {
                    // Fallback if mesh has no UVs to generate tangents from
                    vertex.tangent = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
                }

                vertices.push_back(vertex);
            }

            for (unsigned int j = 0; j < ai_mesh->mNumFaces; j++) {
                aiFace face = ai_mesh->mFaces[j];
                for (unsigned int k = 0; k < face.mNumIndices; k++) {
                    indices.push_back(face.mIndices[k]);
                }
            }

            // Chargement du matériau PBR
            Material mat = loadMeshMaterial(ai_mesh, scene, base_dir);

            // Upload Mesh
            uint32_t mesh = SigelEngine::get().vctx.resourceManager.createMesh(vertices, indices);
            outMeshes.push_back({mesh, mat});
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene, nodeTransform, base_dir, outMeshes);
        }
    }

    std::vector<SubMesh> loadAssimpModel(const std::string& path)
    {
        Assimp::Importer importer;

        std::cout << "\n==========================================" << std::endl;
        status("ASSIMP", "Loading model: " + path);

        const aiScene* scene = importer.ReadFile(path,
            aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_FlipUVs |
            aiProcess_JoinIdenticalVertices);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            throw std::runtime_error("Erreur Assimp : " + std::string(importer.GetErrorString()));
        }

        std::string base_dir = getBaseDir(path);

        std::cout << "[ASSIMP] Total Meshes: " << scene->mNumMeshes
                  << " | Materials: " << scene->mNumMaterials
                  << " | Embedded Textures: " << scene->mNumTextures << std::endl;
        std::cout << "==========================================\n" << std::endl;

        std::vector<SubMesh> meshes;

        processNode(scene->mRootNode, scene, glm::mat4(1.0f), base_dir, meshes);

        std::cout << "\n[ASSIMP] Finished loading model.\n" << std::endl;

        return meshes;
    }
}
