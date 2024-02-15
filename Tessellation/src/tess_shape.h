// SPDX-License-Identifier: BSD-3-Clause
/*
	tess_shape.h

	What is this?
	~~~~~~~~~~~~~
	A class for shapes that can be tessellated.

	License (BSD-3-Clause)
	~~~~~~~~~~~~~~~~~~~~~~

	Copyright (C) 2024 Brandon Blodget

	This software is provided under the BSD 3-Clause License.
	For the full license text, see LICENSE.txt.

	This file is part of the Tessellation project.
*/

#pragma once

// olcUTIL_Geometry2D is not currently used, but it may be useful in the future
//#define OLC_IGNORE_VEC2D
//#include "olcUTIL_Geometry2D.h"

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
 
#define OLC_PGEX_TRANSFORMEDVIEW
#include "olcPGEX_TransformedView.h"

#include <vector>
#include <numeric> // For std::accumulate
#include <cmath>   // For std::round, std::pow

class TessShape {
public:
	TessShape(olc::TransformedView* tv, const std::vector<olc::vf2d>& points)
		: tv_(tv), originalPoints_(points), drawPoints_(points), translation_(0.0f, 0.0f), rotation_(0.0f), dirty_(true) 
	{
		originalCentroid_ = computeCentroid(originalPoints_);
		drawCentroid_ = computeCentroid(originalPoints_);
	}

	virtual ~TessShape() {}


	// Move the shape to a new position
	void moveTo(const olc::vf2d& newPos) {
		translation_ = newPos - originalCentroid_;
		dirty_ = true;
	}

	olc::vf2d getCentroid() {
		if (dirty_) {
			recalculateDrawPoints();
			dirty_ = false;
		}

		return drawCentroid_;
	}

	// Rotate the shape
	void rotate(float angleDegrees) {
		rotation_ += angleDegrees;
		dirty_ = true;
	}

	// Draw the shape
	void draw(olc::Pixel p = olc::WHITE) {
		if (dirty_) {
			recalculateDrawPoints();
			dirty_ = false;
		}

		for (size_t i = 0; i < drawPoints_.size(); ++i) {
			tv_->DrawLine(static_cast<olc::vi2d>(drawPoints_[i]),
				static_cast<olc::vi2d>(drawPoints_[(i + 1) % drawPoints_.size()]),
				p);
		}
	}

	// Get snap points (vertices and midpoints of edges)
	std::vector<olc::vf2d> snapPoints() {
		if (dirty_) {
			recalculateDrawPoints();
			dirty_ = false;
		}

		std::vector<olc::vf2d> snapPoints = drawPoints_; // Add vertices
		for (size_t i = 0; i < drawPoints_.size(); ++i) {
			// Add midpoints of edges
			snapPoints.push_back((drawPoints_[i] + drawPoints_[(i + 1) % drawPoints_.size()]) * 0.5f);
		}
		return snapPoints;
	}

protected:
	olc::TransformedView* tv_;
	std::vector<olc::vf2d> originalPoints_; // Original vertices of the shape
	olc::vf2d originalCentroid_;			   // Original centroid of the shape
	std::vector<olc::vf2d> drawPoints_;     // Transformed vertices for drawing
	olc::vf2d drawCentroid_;                  // Transformed centroid for drawing

	olc::vf2d translation_;                 // Translation vector
	float rotation_;                        // Rotation in degrees
	bool dirty_;                            // Flag to recalculate draw points

	// Compute the centroid of the original polygon
	olc::vf2d computeCentroid(const std::vector<olc::vf2d>& points) const {
		float xSum = std::accumulate(points.begin(), points.end(), 0.0f,
			[](float sum, const olc::vf2d& p) { return sum + p.x; });
		float ySum = std::accumulate(points.begin(), points.end(), 0.0f,
			[](float sum, const olc::vf2d& p) { return sum + p.y; });
		float numPoints = static_cast<float>(points.size());
		return { xSum / numPoints, ySum / numPoints };
	}

	// Recalculate draw points based on rotation and then translation
	void recalculateDrawPoints() {
		drawPoints_.clear();
		float angleRadians = rotation_ * (M_PI / 180.0f);

		// Rotate the original points around the original centroid
		for (auto& point : originalPoints_) {
			float xTranslated = point.x - originalCentroid_.x;
			float yTranslated = point.y - originalCentroid_.y;

			float xRotated = xTranslated * cos(angleRadians) - yTranslated * sin(angleRadians);
			float yRotated = xTranslated * sin(angleRadians) + yTranslated * cos(angleRadians);

			// Translate the point back to its position relative to the original centroid
			xRotated += originalCentroid_.x;
			yRotated += originalCentroid_.y;

			// Round the coordinates to reduce numerical precision errors
			olc::vf2d roundedPoint = RoundPointCoordinates(olc::vf2d(xRotated, yRotated), 2);
			drawPoints_.push_back(roundedPoint);
		}

		// Apply the overall translation to the draw points
		for (auto& point : drawPoints_) {
			point.x += translation_.x;
			point.y += translation_.y;
			point = RoundPointCoordinates(point, 2);
		}

		// Update the draw centroid position after translation
		drawCentroid_.x = originalCentroid_.x + translation_.x;
		drawCentroid_.y = originalCentroid_.y + translation_.y;
		drawCentroid_ = RoundPointCoordinates(drawCentroid_, 2);
	}


	// Round the coordinates of a point to a given number of decimal places
	// i.e. Round the coordinates to 2 decimal places
	// point = RoundPointCoordinates(point, 2);
	olc::vf2d RoundPointCoordinates(const olc::vf2d& point, int decimalPlaces) {
		float multiplier = (float)std::pow(10.0f, decimalPlaces);
		return olc::vf2d(std::round(point.x * multiplier) / multiplier, std::round(point.y * multiplier) / multiplier);
	}
};


