
//--------------------------------------------------------------
//
//  Kevin M. Smith
//
//  Simple Octree Implementation 11/10/2020
// 
//  Copyright (c) by Kevin M. Smith
//  Copying or use without permission is prohibited by law. 
//	


#include "Octree.h"
 


//draw a box from a "Box" class  
//
void Octree::drawBox(const Box &box) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	ofVec3f p = ofVec3f(center.x(), center.y(), center.z());
	float w = size.x();
	float h = size.y();
	float d = size.z();
	ofDrawBox(p, w, h, d);
}

// return a Mesh Bounding Box for the entire Mesh
//
Box Octree::meshBounds(const ofMesh & mesh) {
	int n = mesh.getNumVertices();
	ofVec3f v = mesh.getVertex(0);
	ofVec3f max = v;
	ofVec3f min = v;
	for (int i = 1; i < n; i++) {
		ofVec3f v = mesh.getVertex(i);

		if (v.x > max.x) max.x = v.x;
		else if (v.x < min.x) min.x = v.x;

		if (v.y > max.y) max.y = v.y;
		else if (v.y < min.y) min.y = v.y;

		if (v.z > max.z) max.z = v.z;
		else if (v.z < min.z) min.z = v.z;
	}
	cout << "vertices: " << n << endl;
//	cout << "min: " << min << "max: " << max << endl;
	return Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}

// getMeshPointsInBox:  return an array of indices to points in mesh that are contained 
//                      inside the Box.  Return count of points found;
//
int Octree::getMeshPointsInBox(const ofMesh & mesh, const vector<int>& points,
	Box & box, vector<int> & pointsRtn)
{
	int count = 0;
	for (int i = 0; i < points.size(); i++) {
		ofVec3f v = mesh.getVertex(points[i]);
		if (box.inside(Vector3(v.x, v.y, v.z))) {
			count++;
			pointsRtn.push_back(points[i]);
		}
	}
	return count;
}

// getMeshFacesInBox:  return an array of indices to Faces in mesh that are contained 
//                      inside the Box.  Return count of faces found;
//
int Octree::getMeshFacesInBox(const ofMesh & mesh, const vector<int>& faces,
	Box & box, vector<int> & facesRtn)
{
	int count = 0;
	for (int i = 0; i < faces.size(); i++) {
		ofMeshFace face = mesh.getFace(faces[i]);
		ofVec3f v[3];
		v[0] = face.getVertex(0);
		v[1] = face.getVertex(1);
		v[2] = face.getVertex(2);
		Vector3 p[3];
		p[0] = Vector3(v[0].x, v[0].y, v[0].z);
		p[1] = Vector3(v[1].x, v[1].y, v[1].z);
		p[2] = Vector3(v[2].x, v[2].y, v[2].z);
		if (box.inside(p,3)) {
			count++;
			facesRtn.push_back(faces[i]);
		}
	}
	return count;
}

//  Subdivide a Box into eight(8) equal size boxes, return them in boxList;
//
void Octree::subDivideBox8(const Box &box, vector<Box> & boxList) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	float xdist = (max.x() - min.x()) / 2;
	float ydist = (max.y() - min.y()) / 2;
	float zdist = (max.z() - min.z()) / 2;
	Vector3 h = Vector3(0, ydist, 0);

	//  generate ground floor
	//
	Box b[8];
	b[0] = Box(min, center);
	b[1] = Box(b[0].min() + Vector3(xdist, 0, 0), b[0].max() + Vector3(xdist, 0, 0));
	b[2] = Box(b[1].min() + Vector3(0, 0, zdist), b[1].max() + Vector3(0, 0, zdist));
	b[3] = Box(b[2].min() + Vector3(-xdist, 0, 0), b[2].max() + Vector3(-xdist, 0, 0));

	boxList.clear();
	for (int i = 0; i < 4; i++)
		boxList.push_back(b[i]);

	// generate second story
	//
	for (int i = 4; i < 8; i++) {
		b[i] = Box(b[i - 4].min() + h, b[i - 4].max() + h);
		boxList.push_back(b[i]);
	}
}

void Octree::create(const ofMesh & geo, int numLevels) {
	// initialize octree structure
	//
	mesh = geo;
	int level = 0;
	root.box = meshBounds(mesh);
	if (!bUseFaces) {
		for (int i = 0; i < mesh.getNumVertices(); i++) {
			root.points.push_back(i);
		}
	}
	else {
		// need to load face vertices here
		//
	}

	// recursively build octree
    subdivide(mesh, root, numLevels, level);
	
}


//
// subdivide:  recursive function to perform octree subdivision on a mesh
//
//  subdivide(node) algorithm:
//     1) subdivide box in node into 8 equal side boxes - see helper function subDivideBox8().
//     2) For each child box
//            sort point data into each box  (see helper function getMeshFacesInBox())
//        if a child box contains at least 1 point
//            add child to tree
//            if child is not a leaf node (contains more than 1 point)
//               recursively call subdivide(child)
//         
//      
             
void Octree::subdivide(const ofMesh & mesh, TreeNode & node, int numLevels, int level) {
	// return if all levels have been divided
	if (level >= numLevels) return;

	// subdvide algorithm implemented here
	// 1) subdivide box in node into 8 equal side boxes
	vector<Box> boxList; 
	subDivideBox8(node.box, boxList);

	// 2) for each child box:
	for (Box abox : boxList){
		// sort point data into each box
		vector<int> pointIndices;
		int pointsInBox = getMeshPointsInBox(mesh, node.points, abox, pointIndices);
		// if a child box contains at least 1 point
		if (pointsInBox >= 1) {
			// add child to tree
			node.children.emplace_back();
			TreeNode& child = node.children.back();
			child.box = abox;
			child.points = pointIndices;
			// if child is not a leaf node(contains more than 1 point)
			if (pointsInBox > 1) {
				// recursively call subdivide(child)
				subdivide(mesh, child, numLevels, level + 1);
			}
		}
	}
}

// Implement functions below for Homework project
// 

/* Ray-box intersection */
bool Octree::intersect(const Ray &ray, const TreeNode & node, TreeNode & nodeRtn) {

	// return false if ray does NOT intersect with node's bounding box
	if (!node.box.intersect(ray, 0, FLT_MAX)) {
		return false;
	}

	// if node is a leaf node (contains 1 point)
	if (node.points.size() == 1) {
		// return node in nodeRtn
		nodeRtn = node;
		// return true (end method call)
		return true;
	}

	// else for each child of node,
	for (TreeNode child : node.children) {
		// recursively call intersect(child)
		if (intersect(ray, child, nodeRtn)) {
			// return true once the ray intersects with a decendant's bounding box
			return true;
		}
	}

	// return false if ray doesn't intersect with any children's bounding box
	return false;
}

/* Box-box intersection */
bool Octree::intersect(const Box &box, TreeNode & node, vector<Box> & boxListRtn) {

	// return false if box does NOT intersect with node's bounding box
	if (!node.box.overlap(box)) {
		return false;
	}

	// if node is a leaf node (contains 1 point)
	if (node.points.size() == 1) {
		// return node's bounding box in boxListRtn
		boxListRtn.push_back(node.box);
		// return true (end method call)
		return true;
	}

	bool intersection = false;

	// else for each child,
	for (TreeNode child : node.children) {
		// recursively call intersect(child)
		if (intersect(box, child, boxListRtn)) {
			// set intersection to true if box intersects with a decendant's bounding box
			intersection = true;
		}
	}

	// return true if box intersects with a children's bounding box, otherwise return false
	return intersection;
}

void Octree::draw(TreeNode & node, int numLevels, int level, vector<ofColor> colors) {
	// return if all levels have been drawn
	if (level >= numLevels) {
		return;
	}

	// set color according to level
	ofSetColor(colors[level]);
	// draw level
	drawBox(node.box);
	// increment level
	level++;
	for (TreeNode child : node.children) {
		// recursively call draw(child)
		draw(child, numLevels, level, colors);
	}
}

// Optional
//
void Octree::drawLeafNodes(TreeNode & node) {


}




