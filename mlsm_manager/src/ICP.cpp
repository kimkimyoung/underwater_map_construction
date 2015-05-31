#include "ICP.h"

ICP::ICP(){

}

ICP::ICP(int maxIterations, double errorThreshold){
    maxIterations_ = maxIterations;
    errorThreshold_ = errorThreshold;
}
ICP::ICP(int maxIterations, double errorThreshold, ros::Publisher* debugPublisher){
    maxIterations_ = maxIterations;
    errorThreshold_ = errorThreshold;
    debugPublisher_ = debugPublisher;
}

ICP::~ICP(){

}


void ICP::setDebugPublisher(ros::Publisher* debugPublisher){
    debugPublisher_ = debugPublisher;
}

void ICP::setErrorThreshold(double errorThreshold){
    errorThreshold_ = errorThreshold;
}

void ICP::setMaxIterations(int maxIterations){
    maxIterations_ = maxIterations;
}

void ICP::setWidth(double width){
    width_ = width;
}

void ICP::setSampleStep(double sampleStep){
    sampleStep_ = sampleStep;
}

void ICP::setNSamples(int nSamples){
    nSamples_ = nSamples;
}

// Get the closest points in the direction of estimated movement using a triangle
std::vector<BlockInfo> ICP::closestPoints(intensityCloud::Ptr P, MLSM *X,std::vector<Vector3d> transforms, Vector3d eV){

    std::vector<BlockInfo> result;
    BlockInfo blockInfo;
    mlsm::Block* blockPtr, *bestBlockPtr;
   //intensityCloud::iterator cloudIterator = P->begin();
    cell::iterator cellIterator;
    int i, j;
    int ci, cj;
    cell cellPtr;
    bool found = false;
    double minError = DBL_MAX, d0;
    Vector3d unitSpeed;
    Vector3d unitDir;
    double angle, closestAngle = M_PI;
    //ROS_INFO("Vel %f, %f", eV[0], eV[1]);

    //  if ((eV[0] == 0.0) && (eV[1] == 0.0)){
    //ROS_INFO("No estimation");
    pcl::PointXYZI p0,p[nSamples_];
    for(int i=0;i<P->size();i++){
        if (isnan(P->at(i).x) || isnan(P->at(i).y) || isnan(P->at(i).z)) continue;

        p0 = P->at(i);
        ////ROS_INFO("Point X %.2f Y %.2f Z %.2f", cloudIterator->x, cloudIterator->y, cloudIterator->z);
        if (transforms.size() > 0){
            unitSpeed = eV.normalized();
            for (int j=0;j<nSamples_;j++){
                p[j].x = p0.x + transforms[i][0] * j * sampleStep_;
                p[j].y = p0.y + transforms[i][1] * j * sampleStep_;
                p[j].z = p0.z + transforms[i][2] * j * sampleStep_;
                blockPtr = X->findClosestBlock(p[j]);
                if (blockPtr == NULL) break;

                // Find angle between the block and p0
                unitDir[0] = blockPtr->mean_.x - p0.x;
                unitDir[1] = blockPtr->mean_.y - p0.y;
                unitDir[2] = blockPtr->mean_.z - p0.z;
                unitDir = unitDir.normalized();
                angle = acos(unitSpeed.dot(unitDir));
                if (fabs(angle) < closestAngle){
                    bestBlockPtr = blockPtr;
                    closestAngle = angle;
                }
            }
        }
        else {
            minError = DBL_MAX;
            blockPtr = X->findClosestBlock(p0);
            if (blockPtr == NULL) continue;
            bestBlockPtr = blockPtr;
        }
        blockInfo.blockPtr = bestBlockPtr;
        blockInfo.i = i;
        blockInfo.j = j;
        result.push_back(blockInfo);
        minError = DBL_MAX;
    }
    ////ROS_INFO("Finished finding closest points");
    return result;
    /*}
    //ROS_INFO("With estimation");

    if (eV[0] > eV[1]){
        eV[1] = eV[1] / eV[0];
        eV[0] = 1;
    }
    else{
        eV[0] = eV[0] / eV[1];
        eV[1] = 1;
    }

    double m = 100.0;
    int stepY = 1;
    int stepX = 1;
    int error;
    int errorprev;
    int incY;
    int incX;
    for (;cloudIterator != P->end();cloudIterator++){
        if (isnan(cloudIterator->x) || isnan(cloudIterator->y) || isnan(cloudIterator->z)) continue;
        ////ROS_INFO("Point x = %f, resolution =  %f", cloudIterator->x, X->getResolution());
        i = cloudIterator->x / X->getResolution();
        j = cloudIterator->y / X->getResolution();
        ////ROS_INFO("%d, %d", i, j);
        incY = abs(m * (eV[1] - j));
        incX = abs(m * (eV[0] - i));
        stepY = 1;
        stepX = 1;
        if (incY < 0){
            stepY = -1;
        }
        if (incX < 0){
            stepX = -1;
        }
        if (incX >= incY){
            error = incY - incX;
            errorprev = error;
            while ((i < X->getSpanX())&&(j < X->getSpanY())){
                ////ROS_INFO("%d, %d", i, j);
                if (X->checkCell(i,j)){
                    ////ROS_INFO("CHECK %d, %d", i, j);
                    blockInfo.blockPtr = X->findSuitableBlock(i,j,*cloudIterator);
                    blockInfo.i = i;
                    blockInfo.j = j;
                    result.push_back(blockInfo);
                    break;
                }
                if (error >= 0){
                    j += stepY;
                    errorprev = error;
                    error -= incX;

                    if (error + errorprev < incX){
                        if (X->checkCell(i,j - stepY)){
                            blockInfo.blockPtr = X->findSuitableBlock(i,j,*cloudIterator);
                            blockInfo.i = i;
                            blockInfo.j = j;
                            result.push_back(blockInfo);
                            break;
                        }
                    }
                    if (error + errorprev > incX){
                        if (X->checkCell(i - stepX,j)){
                            blockInfo.blockPtr = X->findSuitableBlock(i,j,*cloudIterator);
                            blockInfo.i = i;
                            blockInfo.j = j;
                            result.push_back(blockInfo);
                            break;
                        }
                    }
                    else {
                        if (X->checkCell(i,j - stepY)){
                            blockInfo.blockPtr = X->findSuitableBlock(i,j,*cloudIterator);
                            blockInfo.i = i;
                            blockInfo.j = j;
                            result.push_back(blockInfo);
                            break;
                        }
                        if (X->checkCell(i - stepX,j)){
                            blockInfo.blockPtr = X->findSuitableBlock(i,j,*cloudIterator);
                            blockInfo.i = i;
                            blockInfo.j = j;
                            result.push_back(blockInfo);
                            break;
                        }
                    }
                }
                i += stepX;
                error += incY;
            }
        }
        else {
            error = incX - incY;
            errorprev = error;
            while ((i < X->getSpanX())&&(j < X->getSpanY())){
                if (X->checkCell(i,j)){
                    blockInfo.blockPtr = X->findSuitableBlock(i,j,*cloudIterator);
                    blockInfo.i = i;
                    blockInfo.j = j;
                    result.push_back(blockInfo);
                    break;
                }
                if (error >= 0){
                    i += stepX;
                    errorprev = error;
                    error -= incY;

                    if (error + errorprev < incY){
                        if (X->checkCell(i - stepX,j)){
                            blockInfo.blockPtr = X->findSuitableBlock(i,j,*cloudIterator);
                            blockInfo.i = i;
                            blockInfo.j = j;
                            result.push_back(blockInfo);
                            break;
                        }
                    }
                    if (error + errorprev > incY){
                        if (X->checkCell(i,j - stepY)){
                            blockInfo.blockPtr = X->findSuitableBlock(i,j,*cloudIterator);
                            blockInfo.i = i;
                            blockInfo.j = j;
                            result.push_back(blockInfo);
                            break;
                        }
                    }
                    else {
                        if (X->checkCell(i,j - stepY)){
                            blockInfo.blockPtr = X->findSuitableBlock(i,j,*cloudIterator);
                            blockInfo.i = i;
                            blockInfo.j = j;
                            result.push_back(blockInfo);
                            break;
                        }
                        if (X->checkCell(i - stepX,j)){
                            blockInfo.blockPtr = X->findSuitableBlock(i,j,*cloudIterator);
                            blockInfo.i = i;
                            blockInfo.j = j;
                            result.push_back(blockInfo);
                            break;
                        }
                    }
                }
                j += stepY;
                error += incX;
            }
        }
    }
    return result;*/
}

std::vector<Vector3d> ICP::applyTransformation(intensityCloud::Ptr P, std::vector<double> timeStamps, Matrix4f R, Vector3d V, Vector3d T){
    std::vector<Vector3d> result;
    intensityCloud::iterator cloudIterator = P->begin();
    //R(0,3) = T[0];
    //R(1,3) = T[1];
    //R(2,3) = T[2];
    //pcl::transformPointCloud(*P,*P,R);
    //ROS_INFO("Speed %f %f %f Stamps %d Points %d",V[0],V[1],V[2], timeStamps.size(), P->size());

    if (((V[0] == 0) && (V[1] == 0) && (V[2] == 0)) || (timeStamps.size() != P->size())){
        R(0,3) = T[0];
        R(1,3) = T[1];
        R(2,3) = T[2];

        pcl::transformPointCloud(*P,*P,R);
    }
    else {
        //double prevStamp = cloudIterator->data_c[3];
        double lastStamp;

        std::vector<double>::reverse_iterator rit = timeStamps.rbegin();
        for (; rit!= timeStamps.rend(); ++rit)
        {
            if (*rit != 0){
                lastStamp = *rit;
                break;
            }
        }

        double movingTime = 0;
        Vector3d transformation;
        for(int i=0;i<P->size();i++){
            if (isnan(P->at(i).x) || isnan(P->at(i).y) || isnan(P->at(i).z)) continue;
            // Possitive stamp means movement. Accumulate translation time

            if (timeStamps[i] != 0) movingTime = lastStamp - timeStamps[i];
            //ROS_INFO("MovingT %f tn = %f ti = %f",movingTime,lastStamp,timeStamps[i]);

            transformation[0] = V[0] * movingTime;
            transformation[1] = V[1] * movingTime;
            transformation[2] = V[2] * movingTime;
            //ROS_INFO("Transformation x = %f y = %f z = %f",transformation[0],transformation[1],transformation[2]);
            // Rotation? (Assuming only linear movement, orientation known
            result.push_back(transformation);
            P->at(i).x = P->at(i).x - transformation[0] + T[0];
            P->at(i).y = P->at(i).y - transformation[1] + T[1];
            P->at(i).z = P->at(i).z - transformation[2] + T[2];
            //prevStamp = fabs(cloudIterator->data_c[3]);

        }
    }
    return result;
}

double ICP::registration(intensityCloud::Ptr P, std::vector<BlockInfo>* Y, Vector3d *Tv, Vector3d *T, Matrix4f *R){
    double error = 0;
    pcl::PointXYZI centroidP = 0, centroidY = 0, centroid2 = 0;
    pcl::registration::TransformationEstimationSVD<pcl::PointXYZI, pcl::PointXYZI> svd;
    pcl::registration::TransformationEstimationLM<pcl::PointXYZI, pcl::PointXYZI> lm;
    pcl::registration::CorrespondenceEstimation<pcl::PointXYZI, pcl::PointXYZI> correspondenceEstimator;



    intensityCloud::iterator cloudIterator = P->begin();
    unsigned int validPoints = 0;
    intensityCloud candidatePointCloud, pointCloud;
    pcl::PointXYZI p;
    int ySize = Y->size();
    //Centroid calculation
    for (;cloudIterator != P->end();cloudIterator++){
        if (isnan(cloudIterator->x) || isnan(cloudIterator->y) || isnan(cloudIterator->z)) continue;
        validPoints++;
        centroidP.x += cloudIterator->x;
        centroidP.y += cloudIterator->y;
        centroidP.z += cloudIterator->z;
    }
    centroidP.x /= validPoints;
    centroidP.y /= validPoints;
    centroidP.z /= validPoints;

    for(int i=0;i<ySize;i++){
        centroidY.x += Y->at(i).blockPtr->mean_.x;
        centroidY.y += Y->at(i).blockPtr->mean_.y;
        centroidY.z += Y->at(i).blockPtr->mean_.z;
    }
    centroidY.x /= Y->size();
    centroidY.y /= Y->size();
    centroidY.z /= Y->size();
    //ROS_INFO("Candidates centroid %f %f %f",centroidY.x,centroidY.y,centroidY.z);
    //ROS_INFO("Points centroid %f %f %f",centroidP.x,centroidP.y,centroidP.z);

    //Move both to same origin
    for(int i=0;i<ySize;i++){
        p.x = Y->at(i).blockPtr->mean_.x - centroidY.x;
        p.y = Y->at(i).blockPtr->mean_.y - centroidY.y;
        p.z = Y->at(i).blockPtr->mean_.z - centroidY.z;
        candidatePointCloud.push_back(p);
    }
    for (cloudIterator = P->begin();cloudIterator != P->end();cloudIterator++){
        if (isnan(cloudIterator->x) || isnan(cloudIterator->y) || isnan(cloudIterator->z)) continue;
        p.x = cloudIterator->x - centroidP.x;
        p.y = cloudIterator->y - centroidP.y;
        p.z = cloudIterator->z - centroidP.z;
        pointCloud.push_back(p);
    }
    // Calculate SVD
    //pcl::ConstCloudIterator<pcl::PointXYZI> p1(pointCloud);
    //pcl::ConstCloudIterator<pcl::PointXYZI> p2(candidatePointCloud);
    pcl::registration::TransformationEstimationSVD<pcl::PointXYZI, pcl::PointXYZI>::Matrix4 rotation;
    rotation = Matrix4f::Identity();
    pcl::PointCloud<pcl::PointXYZI>::Ptr source (new pcl::PointCloud<pcl::PointXYZI>(pointCloud));
    pcl::PointCloud<pcl::PointXYZI>::Ptr target (new pcl::PointCloud<pcl::PointXYZI>(candidatePointCloud));
    boost::shared_ptr<pcl::Correspondences> correspondences (new pcl::Correspondences);
    correspondenceEstimator.setInputSource(source);
    correspondenceEstimator.setInputTarget(target);
    correspondenceEstimator.determineCorrespondences(*correspondences);


    //svd.estimateRigidTransformation(pointCloud,candidatePointCloud, *correspondences,rotation);
    //ROS_INFO("\n \tSVD Rotation \n%f %f %f\n%f %f %f\n%f %f %f",rotation(0,0),rotation(0,1),rotation(0,2),
    //rotation(1,0),rotation(1,1),rotation(1,2),
    //rotation(2,0),rotation(2,1),rotation(2,2));


    //lm.estimateRigidTransformation(pointCloud,candidatePointCloud, *correspondences,rotation);
    ////ROS_INFO("\n \tLM Rotation \n%f %f %f\n%f %f %f\n%f %f %f",rotation(0,0),rotation(0,1),rotation(0,2),
    //                                                                   rotation(1,0),rotation(1,1),rotation(1,2),
    //         rotation(2,0),rotation(2,1),rotation(2,2));
    /*pcl::PointCloud<pcl::PointXYZI>::Ptr source (new pcl::PointCloud<pcl::PointXYZI>(pointCloud));
    pcl::PointCloud<pcl::PointXYZI>::Ptr target (new pcl::PointCloud<pcl::PointXYZI>(candidatePointCloud));
    pcl::IterativeClosestPoint<pcl::PointXYZI, pcl::PointXYZI> icp;

    icp.setInputSource(source);
    icp.setInputTarget(target);
    // Set the max correspondence distance to 5cm (e.g., correspondences with higher distances will be ignored)
    icp.setMaxCorrespondenceDistance (10);
    // Set the maximum number of iterations (criterion 1)
    icp.setMaximumIterations (50);
    // Set the transformation epsilon (criterion 2)
    icp.setTransformationEpsilon (1e-8);
    // Set the euclidean distance difference epsilon (criterion 3)
    icp.setEuclideanFitnessEpsilon (1);
    pcl::PointCloud<pcl::PointXYZI> Final;
    icp.align(Final);
    //std::cout << "has converged:" << icp.hasConverged() << " score: " <<
    //icp.getFitnessScore() << std::endl;
    //std::cout << icp.getFinalTransformation() << std::endl;
    Eigen::Matrix4f rotation = icp.getFinalTransformation ();*/

    //ROS_INFO("TransformPointCloud");
    pcl::transformPointCloud(pointCloud,pointCloud,rotation);

    (*R) = rotation;





    for (cloudIterator = pointCloud.begin();cloudIterator != pointCloud.end();cloudIterator++){
        if (isnan(cloudIterator->x) || isnan(cloudIterator->y) || isnan(cloudIterator->z)) continue;
        cloudIterator->x += centroidP.x;
        cloudIterator->y += centroidP.y;
        cloudIterator->z += centroidP.z;
        centroid2.x += cloudIterator->x;
        centroid2.y += cloudIterator->y;
        centroid2.z += cloudIterator->z;
    }
    centroid2.x /= validPoints;
    centroid2.y /= validPoints;
    centroid2.z /= validPoints;
    //ROS_INFO("\n\tCentroid2 x = %f y = %f z = %f",centroid2.x, centroid2.y, centroid2.z);

    (*T)[0] = centroidY.x - centroidP.x;
    (*T)[1] = centroidY.y - centroidP.y;
    (*T)[2] = centroidY.z - centroidP.z;
    //(*T)[0] = centroidY.x - centroid2.x;
    //(*T)[1] = centroidY.y - centroid2.y;
    //(*T)[2] = centroidY.z - centroid2.z;
    //ROS_INFO("\n\tTranslation x = %f y = %f z = %f error = %f",(*T)[0], (*T)[1], (*T)[2], error);

    return error;
}

double calculateError(intensityCloud::Ptr P, std::vector<BlockInfo>* Y){
    intensityCloud::iterator cloudIterator = P->begin();
    //ROS_INFO("Cloud size %d, Candidates size %d", P->size(), Y->size());
    int i = 0;
    double error = 0;
    for (;cloudIterator != P->end();cloudIterator++){
        if (isnan(cloudIterator->x) || isnan(cloudIterator->y) || isnan(cloudIterator->z)) continue;
        error += sqrt((Y->at(i).blockPtr->mean_.x - cloudIterator->x) * (Y->at(i).blockPtr->mean_.x - cloudIterator->x)
                      + (Y->at(i).blockPtr->mean_.y - cloudIterator->y) * (Y->at(i).blockPtr->mean_.y - cloudIterator->y)
                      + (Y->at(i).blockPtr->mean_.z - cloudIterator->z) * (Y->at(i).blockPtr->mean_.z - cloudIterator->z));
        i++;
    }
    error /= (double)i;
    return error;
}

bool ICP::getTransformation(avora_msgs::StampedIntensityCloudPtr P0, MLSM *X, Vector3d eV, Vector3d eT, Matrix4f eR, Vector3d *Tv, Vector3d *T, Matrix4f *R){
    double error = DBL_MAX;
    unsigned int iterations = 0;
    std::vector<BlockInfo> Y;
    intensityCloud cloud, cloud0;
    pcl::fromROSMsg(P0->cloud,cloud);
    pcl::fromROSMsg(P0->cloud,cloud0);
    intensityCloud::Ptr P = boost::make_shared<intensityCloud>(cloud);
    // Initial transform
    ROS_INFO("Applying initial transformation");
    Vector3d accumulatedT;
    Matrix4f accumulatedR = Matrix<float, 4, 4>::Identity();
    accumulatedT[0] = 0;
    accumulatedT[1] = 0;
    accumulatedT[2] = 0;
    (*Tv)[0] = 0;
    (*Tv)[1] = 0;
    (*Tv)[2] = 0;
    std::vector<Vector3d> transforms = applyTransformation(P, P0->timeStamps, eR, eV, eT);
    ROS_INFO("Initial transformation applied");
    intensityCloud sum;
    sensor_msgs::PointCloud2 debugCloud;
    sum = cloud0 + *P;
    pcl::toROSMsg(sum,debugCloud);
    debugCloud.header.frame_id = P->header.frame_id;
    debugPublisher_->publish(debugCloud);
    while ((iterations < maxIterations_) && (error > errorThreshold_)){
        (*R) = Matrix<float, 4, 4>::Identity();
        // Calculate closest points
        //ROS_INFO("Get closest points");
        Y = closestPoints(P,X, transforms, eV);
        // Calculate transformation
        //ROS_INFO("Get transformation");
        error = registration(P,&Y,Tv, T, R);

        // Iterate through P applying the correct transformation depending on Tv and the stamp
        //ROS_INFO("Applying transformation");
        transforms = applyTransformation(P,P0->timeStamps, *R, *Tv, *T);
        // Calculate error
        //ROS_INFO("Calculate error");
        error = calculateError(P, &Y);
        // Iterate?
        iterations++;
        accumulatedT += *T;
        accumulatedR = accumulatedR * (*R);
        //ROS_INFO("\n%d \tTrl x = %f y = %f z = %f error = %f \n\tAcc  x = %f y = %f z = %f",iterations,(*T)[0], (*T)[1], (*T)[2], error,
        //accumulatedT[0], accumulatedT[1], accumulatedT[2]);

        //ROS_INFO("\n%d \tRotation \n%f %f %f\n%f %f %f\n%f %f %f",iterations,accumulatedR(0,0),accumulatedR(0,1),accumulatedR(0,2),
        //                                                        accumulatedR(1,0),accumulatedR(1,1),accumulatedR(1,2),
        //                                                        accumulatedR(2,0),accumulatedR(2,1),accumulatedR(2,2));

    }
    ROS_INFO("\n%d \tTrl x = %f y = %f z = %f error = %f \n\tAcc  x = %f y = %f z = %f",iterations,(*T)[0], (*T)[1], (*T)[2], error,
             accumulatedT[0], accumulatedT[1], accumulatedT[2]);
    ROS_INFO("\n%d \tRotation \n%f %f %f\n%f %f %f\n%f %f %f",iterations,accumulatedR(0,0),accumulatedR(0,1),accumulatedR(0,2),
             accumulatedR(1,0),accumulatedR(1,1),accumulatedR(1,2),
             accumulatedR(2,0),accumulatedR(2,1),accumulatedR(2,2));

    (*T) = accumulatedT;
    (*R) = accumulatedR;

    return error < errorThreshold_;


}
