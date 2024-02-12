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

class TessShape {
public:
	TessShape(olc::TransformedView* tv, const std::vector<olc::vf2d>& points)
		: tv_(tv), points_(points) {}

	virtual ~TessShape() {}

	// Compute the centroid of the polygon
	olc::vf2d centroid() const {
		float xSum = std::accumulate(points_.begin(), points_.end(), 0.0f,
			[](float sum, const olc::vf2d& p) { return sum + p.x; });
		float ySum = std::accumulate(points_.begin(), points_.end(), 0.0f,
			[](float sum, const olc::vf2d& p) { return sum + p.y; });
		float numPoints = static_cast<float>(points_.size());
		return { xSum / numPoints, ySum / numPoints };
	}

	// Move the shape to a new position
	void moveTo(const olc::vf2d& newPos) {
		olc::vf2d currentCentroid = centroid();
		olc::vf2d delta = newPos - currentCentroid;
		for (auto& point : points_) {
			point += delta;
		}
	}

	// Round the coordinates of a point to a given number of decimal places
	olc::vf2d RoundPointCoordinates(const olc::vf2d& point, int decimalPlaces) {
		float multiplier = (float)std::pow(10.0f, decimalPlaces);
		return olc::vf2d(std::round(point.x * multiplier) / multiplier, std::round(point.y * multiplier) / multiplier);
	}

	// Rotate method in TessShape class
	void rotate(float angleDegrees) {
		olc::vf2d center = centroid();
		float angleRadians = (float)(angleDegrees * (M_PI / 180.0f));

		for (auto& point : points_) { 
			float xTranslated = point.x - center.x;
			float yTranslated = point.y - center.y;

			float xRotated = xTranslated * cos(angleRadians) - yTranslated * sin(angleRadians);
			float yRotated = xTranslated * sin(angleRadians) + yTranslated * cos(angleRadians);

			point.x = xRotated + center.x;
			point.y = yRotated + center.y;

			// Round the coordinates to 2 decimal places
			point = RoundPointCoordinates(point, 2);
		}
	}


	// Draw the shape
	void draw(olc::Pixel p = olc::WHITE) const {
		for (size_t i = 0; i < points_.size(); ++i) {
			tv_->DrawLine(static_cast<olc::vi2d>(points_[i]),
				static_cast<olc::vi2d>(points_[(i + 1) % points_.size()]),
				p);
		}
	}

	// Get snap points (vertices and midpoints of edges)
	std::vector<olc::vf2d> snapPoints() const {
		std::vector<olc::vf2d> snapPoints = points_; // Add vertices
		for (size_t i = 0; i < points_.size(); ++i) {
			// Add midpoints of edges
			snapPoints.push_back((points_[i] + points_[(i + 1) % points_.size()]) * 0.5f);
		}
		return snapPoints;
	}

protected:
	olc::TransformedView* tv_;
	std::vector<olc::vf2d> points_;
};

