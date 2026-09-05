#include "Loader.hpp"
#include "SigelEngine.hpp"

#include <stdexcept>
#include <string>

#include <tiny_obj_loader.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


namespace sigel
{
    std::vector<SubMesh> loadTinyModel(const std::string& path)
    {
		tinyobj::attrib_t                attrib;
		std::vector<tinyobj::shape_t>    shapes;
		std::vector<tinyobj::material_t> materials;
		std::string                      warn, err;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        std::vector<SubMesh> meshes;

        std::string base_dir = "";
        size_t pos = path.find_last_of("/\\");
        if (pos != std::string::npos)
        {
            base_dir = path.substr(0, pos + 1);
        }

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), base_dir.c_str()))
        {
            throw std::runtime_error(warn + err);
        }

        std::cout << materials.size() << " materials found in " << path << std::endl;

        for (const auto &shape : shapes)
        {
            for (const auto &index : shape.mesh.indices)
            {
                Vertex vertex{};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                if (index.normal_index >= 0)
                {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                } else {
                    vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

				auto [it, inserted] = uniqueVertices.insert({vertex, static_cast<uint32_t>(vertices.size())});
				if (inserted)
				{
                    vertices.push_back(vertex);
				}

				indices.push_back(it->second);
            }

            uint32_t mesh = SigelEngine::get().vctx.resourceManager.createMesh(vertices, indices);
            uint32_t texid = SigelEngine::get().vctx.resourceManager.createTextureImage(base_dir + '/' + materials[shape.mesh.material_ids[0]].diffuse_texname);
            meshes.push_back({mesh, texid});
        }
        return meshes;
    }

    static std::string toLower(const std::string& str) {
        std::string lowerStr = str;
        std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
            [](unsigned char c){ return std::tolower(c); });
        return lowerStr;
    }

    std::vector<SubMesh> loadAssimpModel(const std::string& path)
    {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(path,
            aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_FlipUVs |
            aiProcess_JoinIdenticalVertices);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            throw std::runtime_error("Erreur Assimp : " + std::string(importer.GetErrorString()));
        }

        std::vector<SubMesh> meshes;

        std::string base_dir = "";
        size_t pos = path.find_last_of("/\\");
        if (pos != std::string::npos)
        {
            base_dir = path.substr(0, pos + 1);
        }

        std::cout << scene->mNumMaterials << " materials found in " << path << std::endl;

        for (unsigned int i = 0; i < scene->mNumMeshes; i++)
        {
            aiMesh* ai_mesh = scene->mMeshes[i];

            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;

            vertices.reserve(ai_mesh->mNumVertices);

            for (unsigned int j = 0; j < ai_mesh->mNumVertices; j++)
            {
                Vertex vertex{};

                vertex.pos = { ai_mesh->mVertices[j].x, ai_mesh->mVertices[j].y, ai_mesh->mVertices[j].z };

                if (ai_mesh->HasNormals()) {
                    vertex.normal = { ai_mesh->mNormals[j].x, ai_mesh->mNormals[j].y, ai_mesh->mNormals[j].z };
                } else {
                    vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }

                if (ai_mesh->HasTextureCoords(0)) {
                    vertex.texCoord = { ai_mesh->mTextureCoords[0][j].x, ai_mesh->mTextureCoords[0][j].y };
                } else {
                    vertex.texCoord = glm::vec2(0.0f, 0.0f);
                }

                vertices.push_back(vertex);
            }

            for (unsigned int j = 0; j < ai_mesh->mNumFaces; j++)
            {
                aiFace face = ai_mesh->mFaces[j];
                for (unsigned int k = 0; k < face.mNumIndices; k++)
                {
                    indices.push_back(face.mIndices[k]);
                }
            }

            uint32_t texid = 0;
            if (ai_mesh->mMaterialIndex >= 0)
            {
                aiMaterial* material = scene->mMaterials[ai_mesh->mMaterialIndex];
                aiString texPath;
                std::string finalTexPath = "";

                if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                    material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
                    finalTexPath = texPath.C_Str();
                }
                else if (material->GetTextureCount(aiTextureType_BASE_COLOR) > 0) {
                    material->GetTexture(aiTextureType_BASE_COLOR, 0, &texPath);
                    finalTexPath = texPath.C_Str();
                }
                else {
                    bool found = false;
                    for (int type = aiTextureType_NONE + 1; type < aiTextureType_TRANSMISSION; ++type) {
                        aiTextureType aiType = static_cast<aiTextureType>(type);

                        for (unsigned int t = 0; t < material->GetTextureCount(aiType); t++) {
                            material->GetTexture(aiType, t, &texPath);
                            std::string pathStr = texPath.C_Str();
                            std::string lowerPath = toLower(pathStr);

                            if (lowerPath.find("diffuse") != std::string::npos ||
                                lowerPath.find("albedo") != std::string::npos ||
                                lowerPath.find("basecolor") != std::string::npos ||
                                lowerPath.find("base_color") != std::string::npos ||
                                lowerPath.find("col") != std::string::npos) // Parfois juste "nom_col.png"
                            {
                                finalTexPath = pathStr;
                                found = true;
                                break;
                            }
                        }
                        if (found) break;
                    }
                }

                if (!finalTexPath.empty())
                {
                    std::string fullPath = base_dir + finalTexPath;
                    texid = SigelEngine::get().vctx.resourceManager.createTextureImage(fullPath);
                    std::cout << "texture path" << fullPath << std::endl;
                }
            }

            uint32_t mesh = SigelEngine::get().vctx.resourceManager.createMesh(vertices, indices);
            meshes.push_back({mesh, texid});
        }

        return meshes;
    }
}
