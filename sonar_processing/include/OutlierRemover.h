#include <ros/ros.h>
#include <ros/console.h>

#include <nodelet/nodelet.h>

#include <pcl/ros/conversions.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/PCLPointCloud2.h>
#include <string>

#include <avora_msgs/StampedIntensityCloud.h>

#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/filters/radius_outlier_removal.h>

namespace sonar_processing
{
    typedef pcl::PointCloud<pcl::PointXYZI> intensityCloud;
    typedef pcl::PointCloud<pcl::PointXYZRGB> rgbCloud;

    using namespace std;

    class OutlierRemover : public nodelet::Nodelet{
    private:
        //Parameters
        string mode_; //OTSU[PROPORTIONAL], FIXED, PROPORTIONAL, MEAN
        avora_msgs::StampedIntensityCloud stampedCloudMsg_;
        int minNeighbours_;
        double minRadius_;
        int meanK_;
        double stddevMulThresh_;
        bool keepOrganized_;

        //Publishers and subscribers
        ros::Subscriber cloudSubscriber_;
        ros::Publisher cloudPublisher_;
        ros::Publisher rosCloudPublisher_;

        //Topic names
        string cloudSubscribeTopic_;
        string cleanedCloudPublishTopic_;

        ros::NodeHandle nh_;

    public:
        virtual void onInit();
        OutlierRemover();
        ~OutlierRemover();

        void cloudCallback(avora_msgs::StampedIntensityCloudPtr cloudMessagePtr);
        void removeOutliersFromCloud(intensityCloud::Ptr cloudPtr);
        void publishCloud(intensityCloud::Ptr cloudPtr, std::vector<double> timeStamps);
    };
};
