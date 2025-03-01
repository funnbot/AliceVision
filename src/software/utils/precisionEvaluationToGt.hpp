// This file is part of the AliceVision project.
// Copyright (c) 2015 AliceVision contributors.
// Copyright (c) 2012 openMVG contributors.
// This Source Code Form is subject to the terms of the Mozilla Public License,
// v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <aliceVision/numeric/numeric.hpp>
#include <aliceVision/geometry/rigidTransformation3D.hpp>
#include <aliceVision/numeric/BoxStats.hpp>

#include <aliceVision/utils/Histogram.hpp>
#include <dependencies/htmlDoc/htmlDoc.hpp>
#include <dependencies/vectorGraphics/svgDrawer.hpp>

#include <filesystem>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

namespace aliceVision {

/// Compute a 5DOF rigid transform between the two camera trajectories
inline bool computeSimilarity(const std::vector<Vec3>& vec_camPosGT,
                              const std::vector<Vec3>& vec_camPosComputed,
                              std::mt19937& randomNumberGenerator,
                              std::vector<Vec3>& vec_camPosComputed_T,
                              double* Sout,
                              Mat3* Rout,
                              Vec3* tout)
{
    if (vec_camPosGT.size() != vec_camPosComputed.size())
    {
        std::cerr << "Cannot perform registration, vector sizes are different" << std::endl;
        return false;
    }

    // Move input point in appropriate container
    Mat x1(3, vec_camPosGT.size());
    Mat x2(3, vec_camPosGT.size());
    for (size_t i = 0; i < vec_camPosGT.size(); ++i)
    {
        x1.col(i) = vec_camPosComputed[i];
        x2.col(i) = vec_camPosGT[i];
    }
    // Compute rigid transformation p'i = S R pi + t

    double S;
    Vec3 t;
    Mat3 R;
    std::vector<std::size_t> inliers;
    if (!aliceVision::geometry::ACRansac_FindRTS(x1, x2, randomNumberGenerator, S, t, R, inliers, true))
        return false;

    vec_camPosComputed_T.resize(vec_camPosGT.size());
    // std::vector<double> vec_residualErrors(vec_camPosGT.size());
    for (size_t i = 0; i < vec_camPosGT.size(); ++i)
    {
        Vec3 newPos = S * R * (vec_camPosComputed[i]) + t;
        vec_camPosComputed_T[i] = newPos;
        // const double dResidual = (newPos - vec_camPosGT[i]).norm();
        // vec_residualErrors[i] = dResidual;
    }

    *Sout = S;
    *Rout = R;
    *tout = t;
    return true;
}

/// Export to PLY two camera trajectories
inline bool exportToPly(const std::vector<Vec3>& vec_camPosGT, const std::vector<Vec3>& vec_camPosComputed, const std::string& sFileName)
{
    std::ofstream outfile;
    outfile.open(sFileName, std::ios_base::out);

    outfile << "ply" << '\n'
            << "format ascii 1.0" << '\n'
            << "element vertex " << vec_camPosGT.size() + vec_camPosComputed.size() << '\n'
            << "property float x" << '\n'
            << "property float y" << '\n'
            << "property float z" << '\n'
            << "property uchar red" << '\n'
            << "property uchar green" << '\n'
            << "property uchar blue" << '\n'
            << "end_header" << std::endl;

    for (size_t i = 0; i < vec_camPosGT.size(); ++i)
    {
        outfile << vec_camPosGT[i].transpose() << " 0 255 0"
                << "\n";
    }

    for (size_t i = 0; i < vec_camPosComputed.size(); ++i)
    {
        outfile << vec_camPosComputed[i].transpose() << " 255 255 0"
                << "\n";
    }
    outfile.flush();
    bool bOk = outfile.good();
    outfile.close();
    return bOk;
}

/// Compare two camera path (translation and rotation residual after a 5DOF rigid registration)
/// Export computed statistics to a HTLM stream
inline void EvaluteToGT(const std::vector<Vec3>& vec_camCenterGT,
                        const std::vector<Vec3>& vec_camCenterComputed,
                        const std::vector<Mat3>& vec_camRotGT,
                        const std::vector<Mat3>& vec_camRotComputed,
                        const std::string& sOutPath,
                        std::mt19937& randomNumberGenerator,
                        htmlDocument::htmlDocumentStream* htmlDocStream)
{
    const std::size_t numCameras = vec_camCenterGT.size();
    assert(numCameras > 0);
    assert(numCameras == vec_camCenterComputed.size());
    assert(numCameras == vec_camRotGT.size());
    assert(numCameras == vec_camRotComputed.size());

    // Compute global 3D similarity between the camera position
    std::vector<Vec3> vec_camPosComputed_T;
    Mat3 R;
    Vec3 t;
    double scale;

    computeSimilarity(vec_camCenterGT, vec_camCenterComputed, randomNumberGenerator, vec_camPosComputed_T, &scale, &R, &t);

    std::cout << "\nEstimated similarity transformation between the sequences\n";
    std::cout << "R\n" << R << std::endl;
    std::cout << "t\n" << t << std::endl;
    std::cout << "scale\n" << scale << std::endl;

    // Compute statistics and export them
    // -a. distance between camera center
    // -b. angle between rotation matrix

    // -a. distance between camera center
    double trajectoryLength = 0;
    std::vector<double> vec_baselineErrors;
    for (std::size_t i = 0; i < numCameras; ++i)
    {
        const double dResidual = (vec_camCenterGT[i] - vec_camPosComputed_T[i]).norm();
        vec_baselineErrors.push_back(dResidual);
        if (i > 0 && i < numCameras - 2)
            trajectoryLength += (vec_camCenterGT[i] - vec_camCenterGT[i + 1]).norm();
    }

    std::cout << std::endl << "\nTrajectory length: " << trajectoryLength;

    // -b. angle between rotation matrix
    std::vector<double> vec_angularErrors;
    {
        std::vector<Mat3>::const_iterator iter1 = vec_camRotGT.begin();
        for (std::vector<Mat3>::const_iterator iter2 = vec_camRotComputed.begin(); iter2 != vec_camRotComputed.end(); ++iter2, ++iter1)
        {
            const Mat3 R1 = *iter1;                   // GT
            const Mat3 R2T = *iter2 * R.transpose();  // Computed

            const double angularErrorDegree = radianToDegree(getRotationMagnitude(R1 * R2T.transpose()));
            vec_angularErrors.push_back(angularErrorDegree);
        }
    }

    // Display residual errors :
    //  std::cout << "\nBaseline residuals (in GT unit)\n";
    //  copy(vec_baselineErrors.begin(), vec_baselineErrors.end(), std::ostream_iterator<double>(std::cout, " , "));
    //  std::cout << "\nAngular residuals (Degree) \n";
    //  copy(vec_angularErrors.begin(), vec_angularErrors.end(), std::ostream_iterator<double>(std::cout, " , "));

    BoxStats<double> statsBaseline(vec_baselineErrors.begin(), vec_baselineErrors.end());
    std::cout << std::endl << "\nBaseline error statistics:\n" << statsBaseline;

    BoxStats<double> statsAngular(vec_angularErrors.begin(), vec_angularErrors.end());
    std::cout << std::endl << "\nAngular error statistics:\n" << statsAngular;

    // Export camera position (viewable)
    exportToPly(vec_camCenterGT, vec_camPosComputed_T, (fs::path(sOutPath) / "camera_Registered.ply").string());

    exportToPly(vec_camCenterGT, vec_camCenterComputed, (fs::path(sOutPath) / "camera_original.ply").string());

    //-- Export residual to the HTML report
    {
        using namespace htmlDocument;
        const std::string sNewLine = "<br>";
        const std::string sFullLine = "<hr>";

        htmlDocStream->pushInfo(sFullLine);
        htmlDocStream->pushInfo(htmlMarkup("h1", "Compare GT camera position and looking direction."));
        htmlDocStream->pushInfo(" Display per camera after a 3D similarity estimation:<br>");
        htmlDocStream->pushInfo("<ul><li>Baseline_Residual -> localization error of camera center to GT (in GT unit),</li>");
        htmlDocStream->pushInfo("<li>Angular_residuals -> direction error as an angular degree error.</li></ul>");

        htmlDocStream->pushInfo(htmlMarkup("h2", "Baseline errors"));
        std::ostringstream os;
        os << "Baseline_Residual=[";
        std::copy(vec_baselineErrors.begin(), vec_baselineErrors.end(), std::ostream_iterator<double>(os, " "));
        os << "];";
        htmlDocStream->pushInfo(sFullLine);
        htmlDocStream->pushInfo(htmlDocument::htmlMarkup("pre", os.str()));

        os.str("");
        os << "min = " << statsBaseline.min;
        htmlDocStream->pushInfo(sFullLine);
        htmlDocStream->pushInfo(htmlDocument::htmlMarkup("pre", os.str()));

        os.str("");
        os << "max = " << statsBaseline.max;
        htmlDocStream->pushInfo(htmlDocument::htmlMarkup("pre", os.str()));

        os.str("");
        os << "mean = " << statsBaseline.mean;
        htmlDocStream->pushInfo(htmlDocument::htmlMarkup("pre", os.str()));

        os.str("");
        os << "median = " << statsBaseline.median;
        htmlDocStream->pushInfo(htmlDocument::htmlMarkup("pre", os.str()));

        const double maxRange = *std::max_element(vec_baselineErrors.begin(), vec_baselineErrors.end());
        utils::Histogram<double> baselineHistogram(0.0, maxRange, 50);
        baselineHistogram.Add(vec_baselineErrors.begin(), vec_baselineErrors.end());

        svg::svgHisto svg_BaselineHistogram;
        svg_BaselineHistogram.draw(
          baselineHistogram.GetHist(), std::pair<float, float>(0.f, maxRange), (fs::path(sOutPath) / "baseline_histogram.svg").string(), 600, 200);

        os.str("");
        os << sNewLine << "Baseline errors histogram" << sNewLine;
        os << "<img src=\""
           << "baseline_histogram.svg"
           << "\" height=\"300\" width =\"800\">\n";
        htmlDocStream->pushInfo(os.str());

        {
            std::vector<double> xvalues(vec_baselineErrors.size());
            std::iota(xvalues.begin(), xvalues.end(), 0);
            std::pair<std::pair<double, double>, std::pair<double, double>> range = autoJSXGraphViewport<double>(xvalues, vec_baselineErrors);
            range.first.first = 0;
            range.first.second = xvalues.size() + 1;
            htmlDocument::JSXGraphWrapper jsxGraph;
            jsxGraph.init("baselineErrors", 1000, 300);
            jsxGraph.addXYChart(xvalues, vec_baselineErrors, "line,point");
            jsxGraph.UnsuspendUpdate();
            jsxGraph.setViewport(range);
            jsxGraph.close();
            htmlDocStream->pushInfo(jsxGraph.toStr());
        }
        htmlDocStream->pushInfo(sFullLine);

        htmlDocStream->pushInfo(htmlMarkup("h2", "Angular errors"));
        os.str("");
        os << "Angular_residuals=[";
        std::copy(vec_angularErrors.begin(), vec_angularErrors.end(), std::ostream_iterator<double>(os, " "));
        os << "];";
        htmlDocStream->pushInfo("<br>");
        htmlDocStream->pushInfo(htmlDocument::htmlMarkup("pre", os.str()));

        os.str("");
        os << "min = " << statsAngular.min;
        htmlDocStream->pushInfo(sFullLine);
        htmlDocStream->pushInfo(htmlDocument::htmlMarkup("pre", os.str()));

        os.str("");
        os << "max = " << statsAngular.max;
        htmlDocStream->pushInfo(htmlDocument::htmlMarkup("pre", os.str()));

        os.str("");
        os << "mean = " << statsAngular.mean;
        htmlDocStream->pushInfo(htmlDocument::htmlMarkup("pre", os.str()));

        os.str("");
        os << "median = " << statsAngular.median;
        htmlDocStream->pushInfo(htmlDocument::htmlMarkup("pre", os.str()));

        const double maxRangeAngular = *std::max_element(vec_angularErrors.begin(), vec_angularErrors.end());
        utils::Histogram<double> angularHistogram(0.0, maxRangeAngular, 50);
        angularHistogram.Add(vec_angularErrors.begin(), vec_angularErrors.end());

        svg::svgHisto svg_AngularHistogram;
        svg_AngularHistogram.draw(angularHistogram.GetHist(),
                                  std::pair<float, float>(0.f, maxRangeAngular),
                                  (fs::path(sOutPath) / "angular_histogram.svg").string(),
                                  600,
                                  200);

        os.str("");
        os << sNewLine << "Angular errors histogram" << sNewLine;
        os << "<img src=\""
           << "angular_histogram.svg"
           << "\" height=\"300\" width =\"800\">\n";
        htmlDocStream->pushInfo(os.str());

        {
            std::vector<double> xvalues(vec_angularErrors.size());
            std::iota(xvalues.begin(), xvalues.end(), 0);
            std::pair<std::pair<double, double>, std::pair<double, double>> range = autoJSXGraphViewport<double>(xvalues, vec_angularErrors);
            range.first.first = 0;
            range.first.second = xvalues.size() + 1;
            htmlDocument::JSXGraphWrapper jsxGraph;
            jsxGraph.init("AngularErrors", 1000, 300);
            jsxGraph.addXYChart(xvalues, vec_angularErrors, "line,point");
            jsxGraph.UnsuspendUpdate();
            jsxGraph.setViewport(range);
            jsxGraph.close();
            htmlDocStream->pushInfo(jsxGraph.toStr());
        }
        htmlDocStream->pushInfo(sFullLine);
    }
}

// Find a file in a list and return the index, or -1 if nothing found.
// Handle relative/absolute paths
inline int findIdGT(const std::string& file, const std::vector<std::string>& filelist)
{
    int result = -1;
    std::string file_relative = fs::path(file).filename().string();
    for (unsigned int i = 0; i < filelist.size(); ++i)
    {
        if (file_relative.compare(fs::path(filelist[i]).stem().string()) == 0)
        {
            result = i;
            break;
        }
    }

    return result;
}

}  // namespace aliceVision
