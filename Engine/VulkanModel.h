#pragma once

#include <stdlib.h>
#include <string>
#include <fstream>
#include <vector>

#include "vulkan/vulkan.h"

#include <assimp/Importer.hpp> 
#include <assimp/scene.h>     
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "VulkanDevice.h"
#include "VulkanBuffer.h"

#define VERTEX_BUFFER_BIND_ID 0

namespace VEngine
{
	

	typedef enum ModelVertexComponent {
		VERTEX_COMPONENT_POSITION = 0x0,
		VERTEX_COMPONENT_NORMAL = 0x1,
		VERTEX_COMPONENT_COLOR = 0x2,
		VERTEX_COMPONENT_UV = 0x3,
		VERTEX_COMPONENT_TANGENT = 0x4,
		VERTEX_COMPONENT_BITANGENT = 0x5,
		VERTEX_COMPONENT_DUMMY_FLOAT = 0x6,
		VERTEX_COMPONENT_DUMMY_VEC4 = 0x7
	} ModelVertexComponent;



	struct VertexLayout {
	public:

		std::vector<ModelVertexComponent> components;

		VertexLayout(std::vector<ModelVertexComponent> components)
		{
			this->components = std::move(components);
		}

		uint32_t stride()
		{
			uint32_t res = 0;
			for (auto& component : components)
			{
				switch (component)
				{
				case VERTEX_COMPONENT_UV:
					res += 2 * sizeof(float);
					break;
				case VERTEX_COMPONENT_DUMMY_FLOAT:
					res += sizeof(float);
					break;
				case VERTEX_COMPONENT_DUMMY_VEC4:
					res += 4 * sizeof(float);
					break;
				default:
					res += 3 * sizeof(float);
				}
			}
			return res;
		}
	};

	struct ModelCreateInfo {
		glm::vec3 center;
		glm::vec3 scale;
		glm::vec2 uvscale;
		VkMemoryPropertyFlags memoryPropertyFlags = 0;

		ModelCreateInfo() : center(glm::vec3(0.0f)), scale(glm::vec3(1.0f)), uvscale(glm::vec2(1.0f)) {};

		ModelCreateInfo(glm::vec3 scale, glm::vec2 uvscale, glm::vec3 center)
		{
			this->center = center;
			this->scale = scale;
			this->uvscale = uvscale;
		}

		ModelCreateInfo(float scale, float uvscale, float center)
		{
			this->center = glm::vec3(center);
			this->scale = glm::vec3(scale);
			this->uvscale = glm::vec2(uvscale);
		}

	};

	struct Model {
		VkDevice device = nullptr;
		VulkanBuffer vertices;
		VulkanBuffer indices;
		uint32_t indexCount = 0;
		uint32_t vertexCount = 0;

		struct ModelPart {
			uint32_t vertexBase;
			uint32_t vertexCount;
			uint32_t indexBase;
			uint32_t indexCount;
		};
		std::vector<ModelPart> parts;

		static const int defaultFlags = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals;

		struct Dimension
		{
			glm::vec3 min = glm::vec3(FLT_MAX);
			glm::vec3 max = glm::vec3(-FLT_MAX);
			glm::vec3 size;
		} dim;

		void destroy()
		{
			assert(device);
			if (vertices.getBuffer()) {
				vkDestroyBuffer(device, vertices.getBuffer(), nullptr);
				vkFreeMemory(device, vertices.getMemory(), nullptr);
				if (indices.getBuffer() != VK_NULL_HANDLE)
				{
					vkDestroyBuffer(device, indices.getBuffer(), nullptr);
					vkFreeMemory(device, indices.getMemory(), nullptr);
				}
			}
		}

		bool loadFromFile(const std::string& filename, VertexLayout layout, ModelCreateInfo* createInfo, VulkanDevice* device, VkQueue copyQueue)
		{
			this->device = device->getDevice();

			Assimp::Importer Importer;
			const aiScene* pScene;

			// Load file
			pScene = Importer.ReadFile(filename.c_str(), defaultFlags);
			if (!pScene) {
				std::string error = Importer.GetErrorString();
				ELOG("Failed to load object!");
				throw std::runtime_error("Failed to load object!");
			}


			if (pScene)
			{
				parts.clear();
				parts.resize(pScene->mNumMeshes);

				glm::vec3 scale(1.0f);
				glm::vec2 uvscale(1.0f);
				glm::vec3 center(0.0f);
				if (createInfo)
				{
					scale = createInfo->scale;
					uvscale = createInfo->uvscale;
					center = createInfo->center;
				}

				std::vector<float> vertexBuffer;
				std::vector<uint32_t> indexBuffer;

				vertexCount = 0;
				indexCount = 0;

				// Load meshes
				for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
				{
					const aiMesh* paiMesh = pScene->mMeshes[i];

					parts[i] = {};
					parts[i].vertexBase = vertexCount;
					parts[i].indexBase = indexCount;

					vertexCount += pScene->mMeshes[i]->mNumVertices;

					aiColor3D pColor(0.f, 0.f, 0.f);
					pScene->mMaterials[paiMesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, pColor);

					const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

					for (unsigned int j = 0; j < paiMesh->mNumVertices; j++)
					{
						const aiVector3D* pPos = &(paiMesh->mVertices[j]);
						const aiVector3D* pNormal = &(paiMesh->mNormals[j]);
						const aiVector3D* pTexCoord = (paiMesh->HasTextureCoords(0)) ? &(paiMesh->mTextureCoords[0][j]) : &Zero3D;
						const aiVector3D* pTangent = (paiMesh->HasTangentsAndBitangents()) ? &(paiMesh->mTangents[j]) : &Zero3D;
						const aiVector3D* pBiTangent = (paiMesh->HasTangentsAndBitangents()) ? &(paiMesh->mBitangents[j]) : &Zero3D;

						for (auto& component : layout.components)
						{
							switch (component) {
							case VERTEX_COMPONENT_POSITION:
								vertexBuffer.push_back(pPos->x * scale.x + center.x);
								vertexBuffer.push_back(pPos->y * scale.y + center.y);
								vertexBuffer.push_back(pPos->z * scale.z + center.z);
								break;
							case VERTEX_COMPONENT_NORMAL:
								vertexBuffer.push_back(pNormal->x);
								vertexBuffer.push_back(-pNormal->y);
								vertexBuffer.push_back(pNormal->z);
								break;
							case VERTEX_COMPONENT_UV:
								vertexBuffer.push_back(pTexCoord->x * uvscale.s);
								vertexBuffer.push_back(pTexCoord->y * uvscale.t);
								break;
							case VERTEX_COMPONENT_COLOR:
								vertexBuffer.push_back(pColor.r);
								vertexBuffer.push_back(pColor.g);
								vertexBuffer.push_back(pColor.b);
								break;
							case VERTEX_COMPONENT_TANGENT:
								vertexBuffer.push_back(pTangent->x);
								vertexBuffer.push_back(pTangent->y);
								vertexBuffer.push_back(pTangent->z);
								break;
							case VERTEX_COMPONENT_BITANGENT:
								vertexBuffer.push_back(pBiTangent->x);
								vertexBuffer.push_back(pBiTangent->y);
								vertexBuffer.push_back(pBiTangent->z);
								break;
								// Dummy components for padding
							case VERTEX_COMPONENT_DUMMY_FLOAT:
								vertexBuffer.push_back(0.0f);
								break;
							case VERTEX_COMPONENT_DUMMY_VEC4:
								vertexBuffer.push_back(0.0f);
								vertexBuffer.push_back(0.0f);
								vertexBuffer.push_back(0.0f);
								vertexBuffer.push_back(0.0f);
								break;
							};
						}

						dim.max.x = fmax(pPos->x, dim.max.x);
						dim.max.y = fmax(pPos->y, dim.max.y);
						dim.max.z = fmax(pPos->z, dim.max.z);

						dim.min.x = fmin(pPos->x, dim.min.x);
						dim.min.y = fmin(pPos->y, dim.min.y);
						dim.min.z = fmin(pPos->z, dim.min.z);
					}

					dim.size = dim.max - dim.min;

					parts[i].vertexCount = paiMesh->mNumVertices;

					uint32_t indexBase = static_cast<uint32_t>(indexBuffer.size());
					for (unsigned int j = 0; j < paiMesh->mNumFaces; j++)
					{
						const aiFace& Face = paiMesh->mFaces[j];
						if (Face.mNumIndices != 3)
							continue;
						indexBuffer.push_back(indexBase + Face.mIndices[0]);
						indexBuffer.push_back(indexBase + Face.mIndices[1]);
						indexBuffer.push_back(indexBase + Face.mIndices[2]);
						parts[i].indexCount += 3;
						indexCount += 3;
					}
				}


				uint32_t vBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(float);
				uint32_t iBufferSize = static_cast<uint32_t>(indexBuffer.size()) * sizeof(uint32_t);

				// Use staging buffer to move vertex and index buffer to device local memory
				// Create staging buffers
				VulkanBuffer vertexStaging, indexStaging;

				// Vertex staging buffer
				if (device->createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					&vertexStaging,
					vBufferSize,
					vertexBuffer.data()) != VK_SUCCESS) {
					ELOG("Failed to create vertex staging buffer!");
					throw std::runtime_error("Failed to create vertex staging buffer!");
				}

				// Index staging buffer
				if (device->createBuffer(
					VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					&indexStaging,
					iBufferSize,
					indexBuffer.data())) {
					ELOG("Failed to create index staging buffer!");
					throw std::runtime_error("Failed to create index staging buffer!");
				}

				// Create device local target buffers
				// Vertex buffer
				if (device->createBuffer(
					VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | createInfo->memoryPropertyFlags,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					&vertices,
					vBufferSize)) {
					ELOG("Failed to create vertex buffer!");
					throw std::runtime_error("Failed to create vertex buffer!");
				}

				// Index buffer
				if (device->createBuffer(
					VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | createInfo->memoryPropertyFlags,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					&indices,
					iBufferSize)) {
					ELOG("Failed to create index buffer!");
					throw std::runtime_error("Failed to create index buffer!");
				}

				// Copy from staging buffers
				VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

				VkBufferCopy copyRegion{};

				copyRegion.size = vertices.getSize();
				vkCmdCopyBuffer(copyCmd, vertexStaging.getBuffer(), vertices.getBuffer(), 1, &copyRegion);

				copyRegion.size = indices.getSize();
				vkCmdCopyBuffer(copyCmd, indexStaging.getBuffer(), indices.getBuffer(), 1, &copyRegion);

				device->flushCommandBuffer(copyCmd, copyQueue);

				// Destroy staging resources
				vkDestroyBuffer(device->getDevice(), vertexStaging.getBuffer(), nullptr);
				vkFreeMemory(device->getDevice(), vertexStaging.getMemory(), nullptr);
				vkDestroyBuffer(device->getDevice(), indexStaging.getBuffer(), nullptr);
				vkFreeMemory(device->getDevice() , indexStaging.getMemory(), nullptr);

				return true;
			}
			else
			{
				
				printf("Error parsing '%s': '%s'\n", filename.c_str(), Importer.GetErrorString());
				

				return false;
			}
		};


		bool loadFromFile(const std::string& filename, VertexLayout layout, ModelCreateInfo info, VulkanDevice* device, VkQueue copyQueue)
		{
			return loadFromFile(filename, layout, &info, device, copyQueue);
		}
	};

	struct VulkanModelComponent
	{
		Model* model;
		VkPipeline* pipeline;
	};

	static void draw(VulkanModelComponent* comp, VkCommandBuffer cmdBuffer)
	{
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *comp->pipeline);
		vkCmdBindVertexBuffers(cmdBuffer, VERTEX_BUFFER_BIND_ID, 1, &comp->model->vertices.getBuffer(), offsets);
		vkCmdBindIndexBuffer(cmdBuffer, comp->model->indices.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmdBuffer, comp->model->indexCount, 1, 0, 0, 0);
	}
};