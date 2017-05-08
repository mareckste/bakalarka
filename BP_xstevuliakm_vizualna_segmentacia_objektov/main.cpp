/*________________________________________________________________________MAIN_RUNNER________________________________________________________________________*/
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#include <iostream>
#include "DBScan.h"
#include "KinectSensor.h"
#include <algorithm>
/************************************************************************************************/

bool cmdOptionExists(char** begin, char** end, const std::string& option) {
    return std::find(begin, end, option) != end;
}

void printInfo() {
    std::cout << "Nazov programu: 3D DBSCAN superpixel segmentacia"
        << "\nAutor:          Marek Stevuliak"
        << "\nDruh projektu:  Bakalarky projekt, FIIT STU 2017" << std::endl;
}

void printHelp() {
    std::cout << "BP_xstevuliakm_vizualna_segmentacia.exe <parameter>"
        << "\n\nmoznosti <parameter>:"
        << "\n-i:          zobrazenie informacii o programe"
        << "\n-h:          zobrazenie pomocnej prirucky" << std::endl;
}
/************************************************************************************************/
void runSegmentation(short opt) {
    float scale = 0.40f;
    std::cout << "Zvolete skalu obrazu <0 - 1>: ";
    std::cin >> scale;
    KinectSensor sensor;

    

    cv::Mat m(1080, 1920, CV_8UC3);
    cv::Mat m1(1080, 1920, CV_8SC3);
    cv::Mat m2(1080, 1920, CV_16UC1);
    cv::Mat m3(1080, 1920, CV_16UC1);
    cv::Mat segm(static_cast<int>(1080 * scale), static_cast<int>(1920 * scale), CV_16UC1);

    cv::Size size{ static_cast<int>(1920 * scale),
        static_cast<int>(1080 * scale) };

    while (true) {
        sensor.getColorData(m, m2);

        if (sensor.m_mapFlag == true) {                     // if frame obtained
            std::cout << "OK\n\n";

            cv::resize(m, m1, size);                        // scale image
            cv::resize(m2, m3, size);

            DBScan dbs{ m1.rows, m1.cols };                 // init dbscan
            if (opt == 3 || opt == 4)
                m3.data = nullptr;                          // no depth

            dbs.convertToDataPoint(m1, m3);                 // convert data
            dbs.DBScanIteration(20, 5.0, 1000, 3);          // run segmentation
            
            if (opt == 1 || opt == 3) 
                dbs.saveSegmentation(segm);                 // save and label
            else
                dbs.labelBorders(m1);

            break;
        }
    }

    if (opt == 2 || opt == 4) {
        cv::imshow("Segm image", m1);                       // draw result
        cv::waitKey(0);
    }
}
/************************************************************************************************/
int main(int argc, char** argv) {
    
    if (cmdOptionExists(argv, argv + argc, "-h")) {
        printHelp();
    }
    else if (cmdOptionExists(argv, argv + argc, "-i")) {
        printInfo();
    }
    else {
        system("cls");
        std::cout << "Volba:"
            << "\n1 3D Segmentacia do suboru"
            << "\n2 3D Segmentacia na obrazovku"
            <<"\n3 2D Segmentacia do suboru"
            << "\n4 2D Segmentacia na obrazovku"
            << "\n\nZadajte cislo volby: ";
        short opt;
        std::cin >> opt;
        system("cls");
        runSegmentation(opt);
    }

	
    return 0;
}
/************************************************************************************************/
/*________________________________________________________________________MAIN_RUNNER________________________________________________________________________*/