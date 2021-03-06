#include <davinci_pcl_utils/davinci_pcl_utils.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include <math.h>
#include <limits>

using namespace std;

const double PI = 3.14;

void DisplayAllMarkers(DavinciPclUtils davinci_pcl_utils,
    ros::Publisher entryPointPub, ros::Publisher exitPointsPub, ros::Publisher finalPointPub, 
    geometry_msgs::Point entryPoint, double radius) {
    
    cout<<"+++++++debug+ s ++++++"<<davinci_pcl_utils.got_clicked_point()<<endl;
    visualization_msgs::Marker marker, final;
    visualization_msgs::MarkerArray markers;
    int numOfMarkers = 36;
    double angleIncremental = (2*PI/numOfMarkers);
    markers.markers.resize(numOfMarkers);
    for(int i = 0; i < numOfMarkers; i++) {
        marker.header.frame_id = "davinci_endo";
        marker.header.stamp = ros::Time();
        //marker.ns = "my_namespace";
        marker.id = i;
        marker.type = visualization_msgs::Marker::CYLINDER;
        marker.action = visualization_msgs::Marker::ADD;
        // cout << "input offset" <<endl;
        // cin >> radius; 
        marker.pose.position.x = entryPoint.x + radius*cos(i*angleIncremental);
        marker.pose.position.y = entryPoint.y + radius*sin(i*angleIncremental);
        marker.pose.position.z = entryPoint.z;
        marker.pose.orientation.x = 0.0;
        marker.pose.orientation.y = 0.0;
        marker.pose.orientation.z = 0.0;
        marker.pose.orientation.w = 1.0;
        marker.scale.x = 0.001;
        marker.scale.y = 0.001;
        marker.scale.z = 0.0001;
        marker.color.a = 1.0; // Don't forget to set the alpha!
        marker.color.r = 0.0;
        marker.color.g = 1.0;
        marker.color.b = 0.0;
    
        markers.markers[i] = marker;
    }

    // publish the entry point
    marker.pose.position.x = entryPoint.x;
    marker.pose.position.y = entryPoint.y;
    marker.pose.position.z = entryPoint.z;
    marker.color.a = 1.0; // Don't forget to set the alpha!
    marker.color.r = 1.0;
    marker.color.g = 0.0;
    marker.color.b = 0.0;
    entryPointPub.publish(marker);
    
    // publish the exit points
    exitPointsPub.publish(markers);

    final = marker;
    cout<<"+++++++debug+ 0 ++++++"<<davinci_pcl_utils.got_clicked_point()<<endl;
    if(davinci_pcl_utils.got_clicked_point()) {
        ROS_INFO("process select an exit point!");
        cout<<"+++++++debug+ 1 ++++++"<<davinci_pcl_utils.got_clicked_point()<<endl;
        davinci_pcl_utils.reset_got_clicked_point();   // reset the selected-points trigger
        cout<<"+++++++debug+ 2 ++++++"<<davinci_pcl_utils.got_clicked_point()<<endl;

        geometry_msgs::Point clicked_point = davinci_pcl_utils.get_clicked_point().point;

        double closest = DBL_MAX;
        for(int i = 0; i < numOfMarkers; i++) {
            double dist = sqrt( pow(marker.pose.position.x - clicked_point.x, 2) + pow(marker.pose.position.y - clicked_point.y, 2) );
            if(dist < closest) {
                final = markers.markers[i];
            }
            final.color.a = 1.0; // Don't forget to set the alpha!
            final.color.r = 0.0;
            final.color.g = 0.0;
            final.color.b = 1.0;
        }       
    }
    cout<<"+++++++debug+ 3 ++++++"<<davinci_pcl_utils.got_clicked_point()<<endl;
    finalPointPub.publish(marker);
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "davinci_pcl_utils_main"); //node name
    ros::NodeHandle nh;
    DavinciPclUtils davinci_pcl_utils(&nh);

    // whole point cloud received including coordinate information
    while (!davinci_pcl_utils.got_kinect_cloud()) {
        ROS_INFO("did not receive pointcloud");
        ros::spinOnce();
        ros::Duration(1.0).sleep();
    }
    ROS_INFO("got a pointcloud");
    ROS_INFO("saving pointcloud");
    davinci_pcl_utils.save_kinect_snapshot();
    davinci_pcl_utils.save_kinect_clr_snapshot();  // save color version of pointcloud as well


    //set up a publisher to display clouds in rviz:
    ros::Publisher pubCloud = nh.advertise<sensor_msgs::PointCloud2> ("/pcl_cloud_display", 1);
    ros::Publisher thePoint = nh.advertise<geometry_msgs::Point> ("/thePoint", 1);
    ros::Publisher the_plane_normal_pub = nh.advertise<geometry_msgs::Point> ("/the_plane_normal", 1);
    ros::Publisher entryPointPub = nh.advertise<visualization_msgs::Marker>("/entry_point_marker", 0);
    ros::Publisher exitPointsPub = nh.advertise<visualization_msgs::MarkerArray>("/exit_points_markers", 0);
    ros::Publisher finalPointPub = nh.advertise<visualization_msgs::Marker>("/final_point_marker", 0);

    double radius = 0.005;

    //pcl::PointCloud<pcl::PointXYZ> & outputCloud
    pcl::PointCloud<pcl::PointXYZ> display_cloud; // instantiate a pointcloud object, which will be used for display in rviz
    sensor_msgs::PointCloud2 pcl2_display_cloud; //(new sensor_msgs::PointCloud2); //corresponding data type for ROS message
    geometry_msgs::Point theSelectedPoint, thePlaneNormal;

    Eigen::Vector3f centroid, plane_normal, major_axis;
    double plane_dist;

    pcl::PointCloud<pcl::PointXYZ>::Ptr all_points = davinci_pcl_utils.get_all_points();
    pcl::PointCloud<pcl::PointXYZ> cloud;
    //pcl::PointCloud<pcl::PointXYZ> cloud;
    cloud.push_back (pcl::PointXYZ (-1, -1, 0)); 
    cloud.push_back (pcl::PointXYZ (-1, 1, 0)); 
    cloud.push_back (pcl::PointXYZ (1, -1, 0)); 
    cloud.push_back (pcl::PointXYZ (1, 1, 0)); 
    pcl::PointCloud<pcl::PointXYZ>::Ptr test_cloud(&cloud);

    visualization_msgs::Marker final;

    // // compute the three vectors and plane_dist
    // plane_normal = davinci_pcl_utils.get_plane_normal(test_cloud);
    // major_axis = plane_normal;
    // centroid = davinci_pcl_utils.get_centroid(test_cloud);
    // plane_dist = plane_normal.dot(centroid);
    // ROS_INFO_STREAM("ALL POINTS ---------- normal: " << plane_normal.transpose() << "; dist = " << plane_dist);

    // select point(s) and compute the centroid
    while (ros::ok()) {
        if (davinci_pcl_utils.got_selected_points()) {
            ROS_INFO("process selected-points!");
            davinci_pcl_utils.reset_got_selected_points();   // reset the selected-points trigger

            pcl::PointCloud<pcl::PointXYZ>::Ptr selected_points = davinci_pcl_utils.get_selected_points();
            davinci_pcl_utils.print_points(selected_points);

            // theSelectedPoint.x = selected_points->points[0].x;
            // theSelectedPoint.y = selected_points->points[0].y;
            // theSelectedPoint.z = selected_points->points[0].z;

            // ROS_INFO("Press ENTER to check plane_normal of the selected points.");
            // cin.get();
            // compute the three vectors and plane_dist
            plane_normal = davinci_pcl_utils.get_plane_normal(selected_points);
            major_axis = plane_normal;
            centroid = davinci_pcl_utils.get_centroid(selected_points);
            plane_dist = plane_normal.dot(centroid);
            ROS_INFO_STREAM(" normal: " << plane_normal.transpose() << "; dist = " << plane_dist);

            // ROS_INFO("Press ENTER to check coplanar points.");
            // cin.get();
            // extract coplanar points and create a point cloud -- genpurpose_cloud 
            // davinci_pcl_utils.extract_coplanar_pcl_operation(centroid); // offset the transformed, selected points and put result in gen-purpose object
            // pcl::PointCloud<pcl::PointXYZ>::Ptr genpurpose_cloud = davinci_pcl_utils.get_genpurpose_points();
            // // compute the three vectors and plane_dist
            // plane_normal = davinci_pcl_utils.get_plane_normal(genpurpose_cloud);
            // major_axis = plane_normal;
            // centroid = davinci_pcl_utils.get_centroid(genpurpose_cloud);
            // plane_dist = plane_normal.dot(centroid);
            // ROS_INFO_STREAM("--------------------------normal: " << plane_normal.transpose() << "; dist = -------------------------" << plane_dist);

            davinci_pcl_utils.fit_points_to_plane(selected_points, plane_normal, plane_dist);
            ROS_INFO_STREAM("++++++++++++++++++++++++++normal: " << plane_normal.transpose() << "; dist = ++++++++++++++++++++++++++" << plane_dist);

            theSelectedPoint.x = centroid[0];
            theSelectedPoint.y = centroid[1];
            theSelectedPoint.z = centroid[2];

            thePlaneNormal.x = plane_normal[0];
            thePlaneNormal.y = plane_normal[1];
            thePlaneNormal.z = plane_normal[2];          

            davinci_pcl_utils.get_gen_purpose_cloud(display_cloud);
        }

        thePoint.publish(theSelectedPoint);
        the_plane_normal_pub.publish(thePlaneNormal);

        geometry_msgs::Point entryPoint = theSelectedPoint;
        // DisplayAllMarkers(davinci_pcl_utils, entryPointPub, exitPointsPub, finalPointPub, theSelectedPoint, radius);
        // { 
            // cout<<"+++++++debug+ s ++++++"<<davinci_pcl_utils.got_clicked_point()<<endl;
            visualization_msgs::Marker marker;
            visualization_msgs::MarkerArray markers;
            int numOfMarkers = 36;
            double angleIncremental = (2*PI/numOfMarkers);
            markers.markers.resize(numOfMarkers);
            for(int i = 0; i < numOfMarkers; i++) {
                marker.header.frame_id = "davinci_endo";
                marker.header.stamp = ros::Time();
                //marker.ns = "my_namespace";
                marker.id = i;
                marker.type = visualization_msgs::Marker::CYLINDER;
                marker.action = visualization_msgs::Marker::ADD;
                // cout << "input offset" <<endl;
                // cin >> radius; 
                marker.pose.position.x = entryPoint.x + radius*cos(i*angleIncremental);
                marker.pose.position.y = entryPoint.y + radius*sin(i*angleIncremental);
                marker.pose.position.z = entryPoint.z;
                marker.pose.orientation.x = 0.0;
                marker.pose.orientation.y = 0.0;
                marker.pose.orientation.z = 0.0;
                marker.pose.orientation.w = 1.0;
                marker.scale.x = 0.001;
                marker.scale.y = 0.001;
                marker.scale.z = 0.0001;
                marker.color.a = 1.0; // Don't forget to set the alpha!
                marker.color.r = 0.0;
                marker.color.g = 1.0;
                marker.color.b = 0.0;
            
                markers.markers[i] = marker;
            }

            // publish the entry point
            marker.pose.position.x = entryPoint.x;
            marker.pose.position.y = entryPoint.y;
            marker.pose.position.z = entryPoint.z;
            marker.color.a = 1.0; // Don't forget to set the alpha!
            marker.color.r = 1.0;
            marker.color.g = 0.0;
            marker.color.b = 0.0;
            entryPointPub.publish(marker);
            
            // publish the exit points
            exitPointsPub.publish(markers);

            // final = marker;
            // cout<<"+++++++debug+ 0 ++++++"<<davinci_pcl_utils.got_clicked_point()<<endl;
            if(davinci_pcl_utils.got_clicked_point()) {
                ROS_INFO("process select an exit point!");
                // cout<<"+++++++debug+ 1 ++++++"<<davinci_pcl_utils.got_clicked_point()<<endl;
                davinci_pcl_utils.reset_got_clicked_point();   // reset the selected-points trigger
                // cout<<"+++++++debug+ 2 ++++++"<<davinci_pcl_utils.got_clicked_point()<<endl;

                geometry_msgs::Point clicked_point = davinci_pcl_utils.get_clicked_point().point;

                double closest = DBL_MAX;
                final = markers.markers[0];
                for(int i = 0; i < numOfMarkers; i++) {
                    marker = markers.markers[i];
                    double dist = sqrt( pow(marker.pose.position.x - clicked_point.x, 2) + pow(marker.pose.position.y - clicked_point.y, 2) );
                    if(dist < closest) {
                        closest = dist;
                        final.pose.position.x = marker.pose.position.x;
                        final.pose.position.y = marker.pose.position.y;
                        final.pose.position.z = marker.pose.position.z;
                        final.color.a = 1.0; // Don't forget to set the alpha!
                        final.color.r = 1.0;
                        final.color.g = 0.0;
                        final.color.b = 1.0;
                    }
                }
                // finalPointPub.publish(final);     
            }
            finalPointPub.publish(final);
            // cout<<"+++++++debug+ 3 ++++++"<<davinci_pcl_utils.got_clicked_point()<<endl;
        // }


        // //davinci_pcl_utils.get_gen_purpose_cloud(display_cloud);
        // pcl::toROSMsg(display_cloud, pcl2_display_cloud); //convert datatype to compatible ROS message type for publication
        // pcl2_display_cloud.header.stamp = ros::Time::now(); //update the time stamp, so rviz does not complain        
        // pubCloud.publish(pcl2_display_cloud); //publish a point cloud that can be viewed in rviz (under topic pcl_cloud_display)

        ros::Duration(0.5).sleep(); // sleep for half a second
        ros::spinOnce();
    }
    ROS_INFO("my work here is done!");

}

