// SPDX-License-Identifier: BSD-3-Clause
/*
	tess.cpp

	What is this?
	~~~~~~~~~~~~~
	This is a Tessellation application that allows the user to place and rotate
	geometric shapes on the screen. The shapes are triangles, squares, hexagons,
	and darts. The user can zoom in and out, pan the view, and rotate the shapes
	using the mouse and keyboard.

	License (BSD-3-Clause)
	~~~~~~~~~~~~~~~~~~~~~~

	Copyright (C) 2024 Brandon Blodget

	This software is provided under the BSD 3-Clause License.
	For the full license text, see LICENSE.txt.

	This file is part of the Tessellation project.
*/

#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>
#include <array>
#include <vector>
#include <memory>

#include "tess_shape.h"

#include "olcPGEX_TransformedView.h"


// Constants
constexpr float SNAP_DIST_MAX = 5.0f;
constexpr float SIDE_LENGTH = 30.0f;

constexpr float ROTATION_INTEFRVAL = 0.1f;  // Seconds betwen rotations

// A structure that holds two snap points
// the bestCurrentPoint and the bestClosestPoint
// and the distance between them
struct SnapPair
{
	olc::vf2d bestCurrentPoint;
	olc::vf2d bestClosestPoint;
	float distance;
};

// An enum for all the supported shapes
enum class ShapeType
{
	Triangle,
	Square,
	Hexagon,
	IsoQuad,
};



class Tess : public olc::PixelGameEngine
{
public:
	Tess()
	{
		sAppName = "Testing Geometry2D Utilities";
	}

private:
	std::vector<std::unique_ptr<TessShape>> upShapes_;
	std::unique_ptr<TessShape> upCurrentShape_;
	// Pointer to the closest shape to the mouse
	TessShape* pClosestShape_ = nullptr;
	olc::vf2d closestDist_ = { 100000.0f, 100000.0f }; // Initialize with a large value
	SnapPair snapPair_ = { {0.0f, 0.0f}, {0.0f, 0.0f}, 100000.0f };
	ShapeType currentShapeType_ = ShapeType::Triangle;
	olc::TransformedView tv_;
	float timeSinceLastRotation_ = 0.0f;




public:

	bool OnUserCreate() override
	{
		// Initialize TransformedView settings
		tv_.Initialise({ScreenWidth(), ScreenHeight()});
		tv_.SetWorldScale({ 1.0f, 1.0f }); // Set initial zoom level
		tv_.SetWorldOffset({ 0.0f, 0.0f }); // Set initial position

		// CreateNewTriangle(olc::vf2d(0.0f, 0.0f)); // Initial position will be updated immediately
		switch (currentShapeType_)
		{
			case ShapeType::Triangle:
				CreateNewTriangle(olc::vf2d(0.0f, 0.0f)); // Initial position will be updated immediately
				break;
			case ShapeType::Square:
				CreateNewSquare(olc::vf2d(0.0f, 0.0f)); // Initial position will be updated immediately
				break;
			case ShapeType::Hexagon:
				CreateNewHexagon(olc::vf2d(0.0f, 0.0f)); // Initial position will be updated immediately
				break;
			case ShapeType::IsoQuad:
				CreateNewIsoQuad(olc::vf2d(0.0f, 0.0f)); // Initial position will be updated immediately
				break;
		}
		return true;
	}


	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::GREY);

		// Zooming in and out
		if (GetKey(olc::Key::Q).bPressed) {
			tv_.ZoomAtScreenPos(1.1f, { ScreenWidth() / 2, ScreenHeight() / 2 }); // Zoom in at screen center
		}
		if (GetKey(olc::Key::A).bPressed) {
			tv_.ZoomAtScreenPos(0.9f, { ScreenWidth() / 2, ScreenHeight() / 2 }); // Zoom out at screen center
		}


		timeSinceLastRotation_ += fElapsedTime;

		// Rotate the shape with the '<' and '>' keys
		if (GetKey(olc::Key::COMMA).bHeld && timeSinceLastRotation_ >= ROTATION_INTEFRVAL) {
			upCurrentShape_->rotate(-15.0f);
			timeSinceLastRotation_ = 0.0f; // Reset the timer
		}
		if (GetKey(olc::Key::PERIOD).bHeld && timeSinceLastRotation_ >= ROTATION_INTEFRVAL) {
			upCurrentShape_->rotate(15.0f);
			timeSinceLastRotation_ = 0.0f; // Reset the timer
		}

		// Panning
		olc::vf2d panDelta = { 0.0f, 0.0f };
		float panSpeed = 100.0f * fElapsedTime; // Adjust pan speed as necessary
		if (GetKey(olc::Key::LEFT).bHeld) panDelta.x += panSpeed;
		if (GetKey(olc::Key::RIGHT).bHeld) panDelta.x -= panSpeed;
		if (GetKey(olc::Key::UP).bHeld) panDelta.y += panSpeed;
		if (GetKey(olc::Key::DOWN).bHeld) panDelta.y -= panSpeed;

		tv_.MoveWorldOffset(panDelta);


		// Rotate through shapes on 'SPACE' key press
		if (GetKey(olc::Key::SPACE).bPressed) {
			switch (currentShapeType_)
			{
				case ShapeType::Triangle:
					currentShapeType_ = ShapeType::Square;
					CreateNewSquare(upCurrentShape_->getCentroid());
					break;
				case ShapeType::Square:
					currentShapeType_ = ShapeType::Hexagon;
					CreateNewHexagon(upCurrentShape_->getCentroid());
					break;
				case ShapeType::Hexagon:
					currentShapeType_ = ShapeType::IsoQuad;
					CreateNewIsoQuad(upCurrentShape_->getCentroid());
					break;
				case ShapeType::IsoQuad:
					currentShapeType_ = ShapeType::Triangle;
					CreateNewTriangle(upCurrentShape_->getCentroid());
					break;
				default:
					currentShapeType_ = ShapeType::Triangle;
					break;
			}
		}

		olc::vf2d vMouse = tv_.ScreenToWorld(GetMousePos());

		// Place triangle on mouse click
		if (GetMouse(0).bPressed) { // Left mouse button is index 0

			// Snap if close to another triangle
			if (snapPair_.distance < SNAP_DIST_MAX)
			{
				// Snap the current triangle in place
				olc::vf2d translation = snapPair_.bestClosestPoint - snapPair_.bestCurrentPoint;
				upCurrentShape_->moveTo(upCurrentShape_->getCentroid() + translation);
			}

			upShapes_.push_back(std::move(upCurrentShape_)); // Move current triangle to the list

			// Create a new shape at the mouse position
			switch (currentShapeType_)
			{
				case ShapeType::Triangle:
					CreateNewTriangle(vMouse); 
					break;
				case ShapeType::Square:
					CreateNewSquare(vMouse); 
					break;
				case ShapeType::Hexagon:
					CreateNewHexagon(vMouse); 
					break;
				case ShapeType::IsoQuad:
					CreateNewIsoQuad(vMouse); 
					break;
			}
		}

		// Undo last action (remove the last place shape) on right mouse click
		if (GetMouse(1).bPressed) { // Right mouse button is index 1
			if (!upShapes_.empty()) {
				upShapes_.pop_back();
			}
		}

		// Update current triangle position to follow mouse
		if (upCurrentShape_) {
			// More the triangle to the mouse position
			upCurrentShape_->moveTo(vMouse);
		}

		// Rotate current triangle with scroll wheel
		int nMouseWheelDelta = GetMouseWheel();
		if (nMouseWheelDelta > 0) {
			// Rotate counter-clockwise
			upCurrentShape_->rotate(-15.0f);
		}
		else if (nMouseWheelDelta < 0) {
			// Rotate clockwise
			upCurrentShape_->rotate(15.0f);
		}

		closestDist_ = { 100000.0f, 100000.0f }; // Initialize with a large value



		// Draw all placed triangles
		for (const auto& shape : upShapes_) {
			shape->draw(olc::WHITE);
			olc::vf2d dist = vMouse - shape->getCentroid();
			if (dist.mag() < closestDist_.mag()) {
				closestDist_ = dist;
				pClosestShape_ = shape.get();
			}
		}

		// Draw the closest triangle in a different red
		// XXX if (pClosestShape_ && closestDist_.mag() < SNAP_DIST_MAX) {
		// XXX	pClosestShape_->draw(olc::RED);
		// XXX}

		// Draw the current triangle
		if (upCurrentShape_) {
			upCurrentShape_->draw(olc::BLUE); // Draw in different color to distinguish
		}

		// Draw the snap points of the closest triangle
		if (upCurrentShape_ && pClosestShape_)
		{
			std::vector<SnapPair> snapPairs = FindClosestSnapPoints(upCurrentShape_.get(), pClosestShape_);
			float minDistance = 100000.0f;
			for (const auto& sp : snapPairs)
			{
				tv_.FillCircle(sp.bestClosestPoint, 2, olc::YELLOW);
				tv_.FillCircle(sp.bestCurrentPoint, 2, olc::GREEN);
				if (sp.distance < minDistance)
				{
					minDistance = sp.distance;
					snapPair_ = sp;
				}

			}
		}

		return true;

	}

	void CreateNewTriangle(const olc::vf2d& position, float sideLength=SIDE_LENGTH) {
		// Height of the equilateral triangle
		float height = (std::sqrt(3.0f) / 2.0f) * sideLength;

		// Calculate vertices of the equilateral triangle
		olc::vf2d p0 = position + olc::vf2d(0.0f, -2.0f / 3.0f * height); // Top vertex
		olc::vf2d p1 = position + olc::vf2d(-sideLength / 2.0f, height / 3.0f); // Bottom left vertex
		olc::vf2d p2 = position + olc::vf2d(sideLength / 2.0f, height / 3.0f); // Bottom right vertex

		// Create a vector of points and initiaize the current shape
		std::vector<olc::vf2d> points = { p0, p1, p2 };
		upCurrentShape_ = std::make_unique<TessShape>(&tv_,points);
	}

	void CreateNewSquare(const olc::vf2d& position, float sideLength = SIDE_LENGTH) {
		// Calculate half of the side length to position vertices around the center
		float halfSide = sideLength / 2.0f;

		// Calculate vertices of the square
		olc::vf2d p0 = position + olc::vf2d(-halfSide, -halfSide); // Top left vertex
		olc::vf2d p1 = position + olc::vf2d(halfSide, -halfSide);  // Top right vertex
		olc::vf2d p2 = position + olc::vf2d(halfSide, halfSide);   // Bottom right vertex
		olc::vf2d p3 = position + olc::vf2d(-halfSide, halfSide);  // Bottom left vertex

		// Create a vector of points and initialize the current shape
		std::vector<olc::vf2d> points = { p0, p1, p2, p3 };
		upCurrentShape_ = std::make_unique<TessShape>(&tv_, points);
	}

	void CreateNewHexagon(const olc::vf2d& position, float sideLength = SIDE_LENGTH) {
		std::vector<olc::vf2d> points;

		// The angle between the center and any vertex of the hexagon is 60 degrees (pi/3 radians).
		// We loop through all six vertices to calculate their positions.
		for (int i = 0; i < 6; ++i) {
			float angle_rad = (float)(M_PI / 3.0f * i); // Convert angle to radians
			// Calculate the position of each vertex
			olc::vf2d vertex = position + olc::vf2d(cos(angle_rad) * sideLength, sin(angle_rad) * sideLength);
			points.push_back(vertex);
		}

		// Create a new TessShape with the calculated vertices
		upCurrentShape_ = std::make_unique<TessShape>(&tv_, points);
	}

	//void CreateNewDart(const olc::vf2d& position, float sideLength = SIDE_LENGTH) {
	//	std::vector<olc::vf2d> points;

	//	// Assuming the dart is oriented vertically with one tip at 'position'
	//	float height = (float)((std::sqrt(3) / 2) * sideLength); // Height of an equilateral triangle

	//	// Calculate vertices based on the dart shape
	//	olc::vf2d topTip = position; // Top tip of the dart
	//	olc::vf2d rightVertex = { position.x + (sideLength / 2), position.y + height };
	//	olc::vf2d bottomTip = { position.x, position.y + 2 * height };
	//	olc::vf2d leftVertex = { position.x - (sideLength / 2), position.y + height };

	//	// Assemble points in order
	//	points.push_back(topTip);
	//	points.push_back(rightVertex);
	//	points.push_back(bottomTip);
	//	points.push_back(leftVertex);

	//	// Create a new TessShape with these points
	//	upCurrentShape_ = std::make_unique<TessShape>(&tv_, points);
	//}

	//void CreateNewIsoTriangle(const olc::vf2d& position, float sideLength = SIDE_LENGTH) 
	//{
	//	// Calculate the base length using the Law of Sines
	//	// sin(30) / baseLength = sin(75) / sideLength
	//	float baseLength = sideLength * std::sin(M_PI * 30.0 / 180.0) / std::sin(M_PI * 75.0 / 180.0);

	//	// Calculate the height of the triangle using the side length and the 75-degree angle
	//	float height = sideLength * std::sin(M_PI * 75.0 / 180.0);

	//	// Calculate vertices of the isosceles triangle
	//	olc::vf2d p0 = position + olc::vf2d(0.0f, -height); // Top vertex
	//	olc::vf2d p1 = position + olc::vf2d(-baseLength / 2.0f, 0.0f); // Bottom left vertex
	//	olc::vf2d p2 = position + olc::vf2d(baseLength / 2.0f, 0.0f); // Bottom right vertex

	//	// Create a vector of points and initialize the current shape
	//	std::vector<olc::vf2d> points = { p0, p1, p2 };
	//	upCurrentShape_ = std::make_unique<TessShape>(&tv_, points);
	//}

	void CreateNewIsoQuad(const olc::vf2d& position, float sideLength = SIDE_LENGTH) {
		// Calculate the height of the IsoTriangle
		float height = sideLength * std::sin(75.0f * M_PI / 180.0f);

		// Calculate the base of the IsoTriangle
		float base = 2.0f * (sideLength * std::cos(75.0f * M_PI / 180.0f));

		// Calculate vertices of the quadrilateral
		olc::vf2d p0 = position + olc::vf2d(-base / 2.0f, 0.0f); // Left base vertex
		olc::vf2d p1 = position + olc::vf2d(0.0f, -height);      // Top vertex
		olc::vf2d p2 = position + olc::vf2d(base / 2.0f, 0.0f);  // Right base vertex
		olc::vf2d p3 = position + olc::vf2d(0.0f, height);       // Bottom vertex

		// Create a vector of points and initialize the current shape
		std::vector<olc::vf2d> points = { p0, p1, p2, p3 };
		upCurrentShape_ = std::make_unique<TessShape>(&tv_, points);
	}

	

	// A function that takes a pointer to the currentTriangle and a pointer to the closestTriangle
	// and returns a SnapPair structure
	std::vector<SnapPair> FindClosestSnapPoints(TessShape* pCurrentShape, TessShape* pClosestShape) {
		std::vector<SnapPair> snapPairs;

		if (!pCurrentShape || !pClosestShape) {
			return snapPairs;
		}

		auto currentSnapPoints = pCurrentShape->snapPoints();
		auto closestSnapPoints = pClosestShape->snapPoints();

		for (const auto& currentPoint : currentSnapPoints)
		{
			for (const auto& closestPoint : closestSnapPoints)
			{
				float distance = (currentPoint - closestPoint).mag();
				if (distance < SNAP_DIST_MAX)
				{
					snapPairs.push_back({ currentPoint, closestPoint, distance });
				}
			}
		}

		return snapPairs;
	}
};

int main()
{
	Tess demo;
	if (demo.Construct(512, 480, 2, 2))
	{
		demo.Start();
	}

	return 0;
}