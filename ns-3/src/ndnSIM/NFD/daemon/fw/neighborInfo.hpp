#ifndef NDN_CXX_UTIL_NEIGHBORINFO_HPP
#define NDN_CXX_UTIL_NEIGHBORINFO_HPP


#include "neighbor.hpp"
// #include "common.hpp"

// #include <list>
#include <vector>
// #include <tuple>
// #include <set>

/*************/

namespace nfd {
namespace fw {

class UpdateNeighborInfo
{
public:
UpdateNeighborInfo(const uint64_t nodeId, const Neighbor Origneighbor, const Neighbor Modneighbor, const uint64_t timeStamp)
// : m_nodeId (nodeId)
// , m_timestamp (timeStamp)
// , m_original_neighbor (Origneighbor)
// , m_modified_neighbor (Modneighbor)
{
    // NInfoList = std::make_shared<std::vector<UpdateNeighborInfo>>();

    m_nodeId = nodeId;
    m_original_neighbor = Origneighbor;
    m_modified_neighbor = Modneighbor;
    m_timestamp = timeStamp;
    // std::cout << "Original/Modified neighbor inserted... Node: " << nodeId << std::endl;
};



UpdateNeighborInfo()
{
    // NInfoList = std::make_shared<std::vector<UpdateNeighborInfo>>();
    // std::vector<UpdateNeighborInfo> NInfoList = 0;
    // std::cout << "Original/Modified neighbor inserted... Node: " << m_nodeId << std::endl;
};


~UpdateNeighborInfo(){

};

// UpdateNeighborInfo(const UpdateNeighborInfo t)
// // : m_nodeId (nodeId)
// // , m_timestamp (timeStamp)
// // , m_original_neighbor (Origneighbor)
// // , m_modified_neighbor (Modneighbor)
// {
//     m_nodeId = t.getNodeId();
//     m_original_neighbor = t.getOrigneighbor();
//     m_modified_neighbor = t.getModneighbor();
//     m_timestamp = t.getTimestamp();
//     // std::cout << "Original/Modified neighbor inserted... Node: " << m_nodeId << std::endl;
// };


// std::shared_ptr<std::vector<UpdateNeighborInfo>> getNInfoList() {
//     return NInfoList;
// };

std::vector<UpdateNeighborInfo>& getNInfoList() {
    return NInfoList;
};

uint64_t
getNodeId() const
{
    return m_nodeId;
}

uint64_t
getTimestamp() const
{
    return m_timestamp;
}

Neighbor
getOrigneighbor() const
{
    return m_original_neighbor;
}

Neighbor
getModneighbor() const
{
    return m_modified_neighbor;
}



private:
uint64_t m_nodeId;
uint64_t m_timestamp;
Neighbor m_original_neighbor;
Neighbor m_modified_neighbor;

// std::shared_ptr<std::vector<UpdateNeighborInfo>> NInfoList;
std::vector<UpdateNeighborInfo> NInfoList;

// friend std::ostream& operator<<(std::ostream& os, const UpdateNeighborInfo& t) {
//   // for (auto element : neighbor)
//     os << t.m_timestamp << "\n" << t.m_original_neighbor << "\n" << t.m_modified_neighbor << std::endl;
//     return os;
//   }

};
}

}
/*************/
#endif
