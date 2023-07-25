#include "cpu/frame/Frame_Pyramid.h"
#include "cpu/icp/ICP.h"
#include "cpu/tsdf/voxel_grid.h"
#include "cpu/tsdf/kinect_fusion_utility.h"
#include "cpu/mesher/Marching_Cubes.h"

int main(){
    //initiating mesher
    std::unique_ptr<Marching_Cubes> mesher = std::make_unique<Marching_Cubes>();
    
    std::string s_dir = "data/rgbd_dataset_freiburg1_xyz/depth"; 
    auto filenames = kinect_fusion::utility::getPngFilesInDirectory(s_dir);
    
    Frame_Pyramid curr_frame(s_dir + "/" + filenames[0]);
    curr_frame.Depth_Pyramid[0]->save_G_off_format("outputs/point_clouds/G.obj");
    
    //initiating grid
    Eigen::Vector3d gridSize(4,4,4); 
    unsigned int res = 128;
    kinect_fusion::VoxelGrid grid(res ,res ,res ,gridSize, curr_frame.Depth_Pyramid[0]->center_of_mass);
    float mu = 0.02;
    
    //somehow we're getting a problem because of our initial T_gk probably
    grid.updateGlobalTSDF(*curr_frame.Depth_Pyramid[0], mu);
    mesher -> Mesher(grid, 0, "outputs/meshes/mesh_1.off");
    auto T = curr_frame.T_gk;

    for(int file_idx = 0; file_idx < filenames.size(); ++file_idx){
        Frame_Pyramid prev_frame(s_dir + "/" + filenames[file_idx]);
        Frame_Pyramid curr_frame(s_dir + "/" + filenames[file_idx + 1]);
        prev_frame.set_T_gk(T);
        curr_frame.set_T_gk(T);
        
        ICP icp(curr_frame, prev_frame, 0.05f, 0.5f);
        T = icp.pyramid_ICP(false);
        
        grid.updateGlobalTSDF(*curr_frame.Depth_Pyramid[0], mu);
        
        curr_frame.Depth_Pyramid[0]->save_off_format("outputs/point_clouds/pc" +std::to_string(file_idx / 2) + ".obj");
        curr_frame.Depth_Pyramid[0]->save_G_off_format("outputs/point_clouds/pc_G" +std::to_string(file_idx / 2) + ".obj");

        mesher -> Mesher(grid, 0, "outputs/meshes/mesh" + std::to_string(file_idx / 2) + ".off");
    }
}