#pragma once

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <limits>
#include <iterator>
#include <algorithm>

namespace Primitives
{
    struct Material
    {
        glm::vec4 colour;
        float ambient;
        float diffuse;
        float specular;
        float shininess;
        //    float reflective;
        //    float transparency;
        //    float refractiveIndex;
        //
        ////        std::shared_ptr<Pattern> pattern;
        //    bool shadow = true;
    };

    struct Shape
    {
        glm::mat4 inverseTransform;
        alignas(16) Material material;
        glm::vec4 data[6];
        uint32_t typeEnum;
    };

    struct NodeTLAS
    {
        glm::vec4 first;
        glm::vec4 second;
    };

    struct NodeBLAS
    {
        glm::vec4 point1;
        glm::vec4 point2;
        glm::vec4 point3;
        glm::vec4 normal1;
        glm::vec4 normal2;
        glm::vec4 normal3;
    };

    struct Mesh
    {
        glm::mat4 inverseTransform;
        Material material;
        //    char padding[12];

        NodeBLAS nodes[1];
    };

    struct BVH
    {
        glm::mat4 inverseTransform;
        alignas(16) Material material;
        NodeTLAS TLAS[1]; // bounding parameters
        //    NodeBLAS BLAS[1]; // shape primitives
    };

    struct Camera
    {
        glm::mat4 inverseTransform;
        float pixelSize;
        float halfWidth;
        float halfHeight;
        uint32_t width;
        uint32_t height;
    };

    Camera makeCamera(glm::vec4 position, glm::vec4 centre, glm::vec4 up, uint32_t hsize,
                      uint32_t vsize, float fov)
    {
        Camera camera{};
        camera.width = hsize;
        camera.height = vsize;

        glm::mat4 transform = glm::lookAt(glm::vec3(position),
                                          glm::vec3(centre), glm::vec3(up));
        camera.inverseTransform = glm::affineInverse(transform);

        float halfView = glm::tan(fov / 2);
        float aspect = (float)hsize / (float)vsize;

        if (aspect >= 1)
        {
            camera.halfWidth = halfView;
            camera.halfHeight = halfView / aspect;
        }
        else
        {
            camera.halfWidth = halfView * aspect;
            camera.halfHeight = halfView;
        }
        camera.pixelSize = (camera.halfWidth * 2.0) / (float)hsize;

        return camera;
    }

    //static int sNextId = 0;
    //uint32_t getNextId() { return ++sNextId; }

    Shape makeSphere(Material &material, glm::mat4 &transform)
    {
        Shape shape{};
        shape.typeEnum = 0;
        shape.inverseTransform = glm::affineInverse(transform);
        shape.material = material;

        return shape;
    }

    Shape makePlane(Material &material, glm::mat4 &transform)
    {
        Shape shape{};
        shape.typeEnum = 1;
        shape.inverseTransform = glm::affineInverse(transform);
        shape.material = material;

        return shape;
    }

    Shape makeTriangle(std::vector<glm::vec4> &params, Material &material, glm::mat4 &transform)
    {
        Shape shape{};
        shape.typeEnum = 2;
        shape.inverseTransform = glm::affineInverse(transform);
        shape.material = material;

        for (int i = 0; i < 6; i++)
        {
            shape.data[i] = params.at(i);
        }

        return shape;
    }

    std::vector<NodeBLAS> parseObjFile(std::string const &path)
    {
        std::string line;

        std::ifstream in(path, std::ios::in);

        if (!in)
        {
            std::cout << "Impossible to open the file: " << path << std::endl;
        }

        std::vector<unsigned int> vertexIndices;
        //    std::vector<unsigned int> uvIndices;
        std::vector<unsigned int> normalIndices;
        std::vector<glm::vec4> temp_vertices;
        // std::vector<glm::dvec2> temp_uvs;
        std::vector<glm::vec4> temp_normals;

        while (std::getline(in, line))
        {
            if (line.substr(0, 2) == "v ")
            {
                std::istringstream v(line.substr(2));
                glm::vec4 vertex;

                double x, y, z;
                v >> x;
                v >> y;
                v >> z;

                vertex = glm::vec4(x, y, z, 1.f);
                temp_vertices.push_back(vertex);
            }
            else if (line.substr(0, 2) == "vn")
            {
                std::istringstream vn(line.substr(3));
                glm::vec4 normal;

                double x, y, z;
                vn >> x;
                vn >> y;
                vn >> z;

                normal = glm::vec4(x, y, z, 0.f);
                temp_normals.push_back(normal);
            }
            else if (line.substr(0, 2) == "f ")
            {
                unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
                std::istringstream f(line.substr(2));

                if (line.find('/') != std::string::npos)
                {
                    for (int i = 0; i < 3; i++)
                    {
                        std::string vStr, vtStr, vnStr;
                        std::getline(f, vStr, '/');
                        std::getline(f, vtStr, '/');
                        std::getline(f, vnStr, ' ');

                        vertexIndex[i] = atoi(vStr.c_str());
                        // uvIndex[i] = atoi(vtStr.c_str());
                        normalIndex[i] = atoi(vnStr.c_str());
                    }
                }
                else
                {
                    std::string vStr[3];
                    std::getline(f, vStr[0], ' ');
                    std::getline(f, vStr[1], ' ');
                    std::getline(f, vStr[2], ' ');
                    vertexIndex[0] = atoi(vStr[0].c_str());
                    vertexIndex[1] = atoi(vStr[1].c_str());
                    vertexIndex[2] = atoi(vStr[2].c_str());
                }

                vertexIndices.push_back(vertexIndex[0]);
                vertexIndices.push_back(vertexIndex[1]);
                vertexIndices.push_back(vertexIndex[2]);
                // uvIndices.push_back(uvIndex[0]);
                // uvIndices.push_back(uvIndex[1]);
                // uvIndices.push_back(uvIndex[2]);
                normalIndices.push_back(normalIndex[0]);
                normalIndices.push_back(normalIndex[1]);
                normalIndices.push_back(normalIndex[2]);
            }
            else if (line.substr(0, 2) == "vt")
            {
                // std::istringstream vt(line.substr(3));
                // glm::dvec2 uv;
                // int U, V;
                // vt >> U;
                // vt >> V;
                // uv = glm::dvec2(U, V);
                // temp_uvs.push_back(uv);
            }
        }

        std::vector<NodeBLAS> triangleParams;
        triangleParams.reserve((vertexIndices.size() / 3));

        for (unsigned int i = 0; i < vertexIndices.size(); i += 3)
        {
            NodeBLAS nextNode = {};

            if (temp_normals.empty())
            {
                glm::vec3 e1 = temp_vertices[vertexIndices[i + 1] - 1] - temp_vertices[vertexIndices[i] - 1];
                glm::vec3 e2 = temp_vertices[vertexIndices[i + 2] - 1] - temp_vertices[vertexIndices[i] - 1];
                glm::vec4 normal = glm::vec4(glm::normalize(glm::cross(e2, e1)), 0.0);

                nextNode = NodeBLAS{
                    temp_vertices[vertexIndices[i] - 1],
                    temp_vertices[vertexIndices[i + 1] - 1],
                    temp_vertices[vertexIndices[i + 2] - 1],
                    normal,
                    normal,
                    normal};
            }
            else
            {
                nextNode = NodeBLAS{
                    temp_vertices[vertexIndices[i] - 1],
                    temp_vertices[vertexIndices[i + 1] - 1],
                    temp_vertices[vertexIndices[i + 2] - 1],
                    temp_normals[normalIndices[i] - 1],
                    temp_normals[normalIndices[i + 1] - 1],
                    temp_normals[normalIndices[i + 2] - 1]

                };
            }

            triangleParams.push_back(nextNode);
            //        triangleParams.push_back(temp_vertices[vertexIndices[i] - 1]);
            //        triangleParams.push_back(temp_vertices[vertexIndices[i + 1] - 1]);
            //        triangleParams.push_back(temp_vertices[vertexIndices[i + 2] - 1]);
            //        triangleParams.push_back(temp_normals[normalIndices[i] - 1]);
            //        triangleParams.push_back(temp_normals[normalIndices[i + 1] - 1]);
            //        triangleParams.push_back(temp_normals[normalIndices[i + 2] - 1]);
        }

        return triangleParams;
    }

    Mesh *makeMesh(std::string const &path, Material &material, glm::mat4 &transform, size_t &size)
    {
        std::vector<NodeBLAS> triangleParams = parseObjFile(path);

        size_t sizeParams = triangleParams.size() * sizeof(NodeBLAS);
        size = sizeof(Mesh) + sizeParams;

        char *ptr = new char[sizeof(Mesh) - 1 + sizeParams];
        Mesh *mesh = reinterpret_cast<Mesh *>(ptr);
        mesh->inverseTransform = glm::affineInverse(transform);
        mesh->material = material;
        memcpy(mesh->nodes, triangleParams.data(), sizeParams);

        //    size += sizeof(Mesh);

        return mesh;
    }

    NodeTLAS mergeBounds(const NodeTLAS &b1, const NodeTLAS &b2)
    {
        NodeTLAS ret{glm::vec4(std::min(b1.first.x, b2.first.x),
                               std::min(b1.first.y, b2.first.y),
                               std::min(b1.first.z, b2.first.z), 1.),
                     glm::vec4(std::max(b1.second.x, b2.second.x),
                               std::max(b1.second.y, b2.second.y),
                               std::max(b1.second.z, b2.second.z), 1.)};

        return ret;
    }

    NodeTLAS blasBounds(const NodeBLAS node)
    {
        glm::vec4 min(std::min({node.point1.x, node.point2.x, node.point3.x}),
                      std::min({node.point1.y, node.point2.y, node.point3.y}),
                      std::min({node.point1.z, node.point2.z, node.point3.z}),
                      1.);

        glm::vec4 max(std::max({node.point1.x, node.point2.x, node.point3.x}),
                      std::max({node.point1.y, node.point2.y, node.point3.y}),
                      std::max({node.point1.z, node.point2.z, node.point3.z}),
                      1.);

        return NodeTLAS{min, max};
    }

    glm::vec4 boundsCentroid(const NodeBLAS &shape)
    {
        return .5f * blasBounds(shape).first + .5f * blasBounds(shape).second;
    }

    void recursiveBuild(std::vector<NodeTLAS> &tlas, std::vector<NodeBLAS> &blas, std::vector<NodeBLAS> &triangleParamsUnsorted, uint32_t level, uint32_t branch, uint32_t start, uint32_t end, uint32_t tlasHeight)
    {
        //    uint32_t node = (std::pow(2, level) - 1) + branch;
        NodeBLAS emptyNode{glm::vec4(-1.f), glm::vec4(-1.f), glm::vec4(-1.f), glm::vec4(-1.f), glm::vec4(-1.f), glm::vec4(-1.f)};
        NodeTLAS emptyBounds{glm::vec4(std::numeric_limits<float>::min()), glm::vec4(std::numeric_limits<float>::min())};
//        NodeTLAS totalBounds{glm::vec4(std::numeric_limits<float>::min()), glm::vec4(std::numeric_limits<float>::max())};

        NodeTLAS centroidBounds{
            glm::vec4(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), 1.f),
            glm::vec4(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), 1.f)};

        for (auto it = triangleParamsUnsorted.begin() + start; it != triangleParamsUnsorted.begin() + end; ++it)
        {
            centroidBounds = mergeBounds(centroidBounds, blasBounds(*it));
        };

        glm::vec4 diagonal = centroidBounds.second - centroidBounds.first;
        uint32_t splitDimension;

        if (diagonal.x > diagonal.y && diagonal.x > diagonal.z)
            splitDimension = 0;
        else if (diagonal.y > diagonal.z)
            splitDimension = 1;
        else
            splitDimension = 2;

        //        int mid = (start + end) / 2;

        if (centroidBounds.first[splitDimension] == centroidBounds.second[splitDimension])
        {
            //            TODO figure out if this bit is necessary.
            //            for (int i = start; i < end; ++i)
            //            {
            //                //                node->addChild(shapes.at(i));
            //
            //            }
            return;
        }
        else
        {
            int mid = (start + end) / 2;
            std::nth_element(&triangleParamsUnsorted[start], &triangleParamsUnsorted[mid],
                             &triangleParamsUnsorted[end-1]+1,
                             [splitDimension](const NodeBLAS &a, const NodeBLAS &b) {
                                 return boundsCentroid(a)[splitDimension] < boundsCentroid(b)[splitDimension];
                             });

            uint32_t node = std::pow(2, level) + branch - 1;
            tlas.at(node) = centroidBounds;
            
            int nShapes = end - start;

            if (nShapes > 2)
            {
                recursiveBuild(tlas, blas, triangleParamsUnsorted, level + 1, branch * 2, start, mid, tlasHeight);
                recursiveBuild(tlas, blas, triangleParamsUnsorted, level + 1, (branch * 2) + 1, mid, end, tlasHeight); //TODO check branch calc
            }
            else
            {
                blas.push_back(triangleParamsUnsorted.at(start));
                //
                if (nShapes == 2)
                {
                    blas.push_back(triangleParamsUnsorted.at(start + 1));
                }
                else
                {
                    //          If there is only one leaf node, just add it twice to make all inner nodes have 2 children
                    //                    blas.push_back(triangleParamsUnsorted.at(start));
                    //                  Instead of adding it twice, add node with w value 0 and check in shader
                    blas.push_back(emptyNode);

                    //                  TODO: need to use sorted triangleParams array instead of this push approach, but also need to track emptynodes

                    //                paddingIndices.push_back(((node-offset)*2)+1);
                }

                //            In case on unbalanced tree, add dummy branches
                if (level < tlasHeight)
                {
                    level += 1;
                    branch *= 2;

                    uint32_t dummyNode = std::pow(2, level) + branch - 1;

                    tlas.at(dummyNode) = centroidBounds;
                    tlas.at(dummyNode + 1) = emptyBounds;

                    blas.push_back(emptyNode);
                    blas.push_back(emptyNode);
                    
                }

                return;
            }
        }

        return;
    }

    uint32_t nextPowerOfTwo(uint32_t v)
    {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;
        return v;
    }

    static uint32_t log2int(uint32_t val)
    {
        if (val == 0)
            return std::numeric_limits<uint32_t>::max();
        if (val == 1)
            return 0;
        uint32_t ret = 0;
        while (val > 1)
        {
            val >>= 1;
            ret++;
        }
        return ret;
    }

    std::vector<NodeTLAS> buildTLAS(std::vector<NodeBLAS> &blas, std::vector<NodeBLAS> &triangleParamsUnsorted)
    {
        std::vector<NodeTLAS> tlas;
        tlas.resize(nextPowerOfTwo(triangleParamsUnsorted.size()) - 1);

        uint32_t tlasHeight = log2int(triangleParamsUnsorted.size());

        //    size_t offset = std::pow(2,std::floor(log2(tlas.size())))-1;

        recursiveBuild(tlas, blas, triangleParamsUnsorted, 0, 0, 0, triangleParamsUnsorted.size(), tlasHeight);

        return tlas;
    }

    std::pair<BVH *, std::vector<NodeBLAS>> makeBVH(std::string const &path, Material &material, glm::mat4 &transform, size_t &size)
    {
        std::vector<NodeBLAS> triangleParamsUnsorted = parseObjFile(path);
        size_t blasSizeParams = triangleParamsUnsorted.size() * sizeof(NodeBLAS);

        std::vector<NodeBLAS> blas;
        blas.reserve(nextPowerOfTwo(triangleParamsUnsorted.size()) - 1);

        std::vector<NodeTLAS> tlas = buildTLAS(blas, triangleParamsUnsorted);
        size_t tlasSizeParams = tlas.size() * sizeof(NodeTLAS);

        //    NodeBLAS emptyNode {glm::vec4(0.f),glm::vec4(0.f),glm::vec4(0.f),glm::vec4(0.f),glm::vec4(0.f),glm::vec4(0.f)};
        //    for (auto idx: paddingIndices) {
        //        auto it = triangleParamsUnsorted.begin();
        //        std::advance(it,idx);
        //        triangleParamsUnsorted.insert(it, emptyNode);
        //    }

        //    size = sizeof(BVH) + tlasSizeParams;
        size = sizeof(BVH) - sizeof(NodeTLAS) + tlasSizeParams;

        char *ptr = new char[size];
        BVH *bvh = reinterpret_cast<BVH *>(ptr);
        bvh->inverseTransform = glm::affineInverse(transform);
        bvh->material = material;

        memcpy(bvh->TLAS, tlas.data(), tlasSizeParams);

        return {bvh, blas};
    }

} // namespace Primitives
