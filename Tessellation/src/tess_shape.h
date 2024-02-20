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

//#define OLC_PGEX_GRAPHICS2D
//#include "olcPGEX_Graphics2D.h"


#include <vector>
#include <numeric> // For std::accumulate
#include <cmath>   // For std::round, std::pow

class TessShape {
public:
	TessShape(olc::TransformedView* tv, const std::vector<olc::vf2d>& points)
		: tv_(tv), originalPoints_(points), drawPoints_(points), translation_(0.0f, 0.0f), rotation_(0.0f), dirty_(true) , color_(olc::BLANK), fill_(false)
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
		// Normalize rotation to be within -360 to +360 degrees
		if (rotation_ > 360.0f) {
			rotation_ -= 360.0f;
		}
		else if (rotation_ < -360.0f) {
			rotation_ += 360.0f;
		}
		dirty_ = true;
	}

	// Get the current rotation angle in degrees
	float getRotation() const {
		return rotation_;
	}


	// Draw the shape
	void draw(olc::Pixel p = olc::WHITE, int thickness=3) {
		if (dirty_) {
			recalculateDrawPoints();
			dirty_ = false;
		}

		// Fill the shape with fill color
		if (fill_)
		{
			for (size_t i = 0; i < drawPoints_.size() - 1; ++i) {
				drawFilledTriangle(drawPoints_[0], drawPoints_[i], drawPoints_[i + 1], color_);
			}
		}

		// Draw the outline of the shape
		//for (size_t i = 0; i < drawPoints_.size(); ++i) {
		//	tv_->DrawLine(drawPoints_[i], drawPoints_[(i + 1) % drawPoints_.size()], p);
		//}

		// Draw the outline of the shape with thicker lines
		for (size_t i = 0; i < drawPoints_.size(); ++i) {
			olc::vf2d p1 = drawPoints_[i];
			olc::vf2d p2 = drawPoints_[(i + 1) % drawPoints_.size()];

			// Calculate the normal vector to the line
			olc::vf2d normal = (p2 - p1).perp().norm();

			// Draw multiple lines with offsets to create a thicker line
			for (int j = -thickness / 2; j <= thickness / 2; ++j) {
				olc::vf2d offset = normal * static_cast<float>(j);
				tv_->DrawLine(p1 + offset, p2 + offset, p);
			}
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

	void setColor(const olc::Pixel& newColor) {
		color_ = newColor;
		if (color_ == olc::BLANK) {
			fill_ = false;
		}
		else {
			fill_ = true;
		}
	}

	// Check if a point is inside the shape
	// This requires the shape vertices to be in ccw or cw order
	bool isInside(const olc::vf2d& point) const {
		int intersections = 0;
		for (size_t i = 0; i < drawPoints_.size(); ++i) {
			size_t j = (i + 1) % drawPoints_.size();
			// Check if the line segment from drawPoints_[i] to drawPoints_[j] intersects with the ray from the point to the right
			if (((drawPoints_[i].y > point.y) != (drawPoints_[j].y > point.y)) &&
				(point.x < (drawPoints_[j].x - drawPoints_[i].x) * (point.y - drawPoints_[i].y) / (drawPoints_[j].y - drawPoints_[i].y) + drawPoints_[i].x)) {
				intersections++;
			}
		}
		return (intersections % 2) == 1;
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
	olc::Pixel color_;                      // Color of the shape
	bool fill_ = false;                     // Fill the shape with color

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

	void drawFilledTriangle(const olc::vf2d& p1, const olc::vf2d& p2, const olc::vf2d& p3, olc::Pixel color) 
	{
		tv_->FillTriangle(p1, p2, p3, color);
	}

};


