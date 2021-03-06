//
// This file is part of Luola2.
// Copyright (C) 2012 Calle Laakkonen
//
// Luola2 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Luola2 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Luola2.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef LUOLA_TERRAIN_POLYGON_H
#define LUOLA_TERRAIN_POLYGON_H

#include <vector>

#include "bounds.h"

namespace terrain {

 /**
  * A convex polygon. The terrain is made up of these.
  */
 class ConvexPolygon {
 public:
     ConvexPolygon() = default;

     /**
      * Construct a convex polygon.
      *
      * The winding order is counterclockwise.
      * @param points the list of vertices. Must contain at least three.
      */
     ConvexPolygon(const Points &point);

     /**
      * Partition an arbitrary polygon into a set of convex polygons.
      *
      * @param points points of a possibly concave polygon
      * @param polys the list to which the polygon partitions will be added
      */
     static void make(const Points &points, std::vector<ConvexPolygon> &polys);

     /**
      * Check for a collision with a moving circle.
      *
      * If a collision is detected, true is returned
      * and the parameters cp and normal will be set
      * to the point of collision and edge normal.
      *
      * @param p circle center point
      * @param r circle radius
      * @param v circle displacement in this timestep
      * @param cp circle's center point at collision
      * @param normal normal of the colliding edge
      * @return true if collision happens
      */
     bool circleCollision(const Point &p, float r, const glm::vec2 &v, Point &cp, glm::vec2 &normal) const;

     /**
      * Check if the given point is inside the polygon.
      *
      * @param point point coordinates
      * @return true if point is inside the polygon
      */
     bool hasPoint(const Point &point) const;

     /**
      * Check if the given polygon is completely inside this one.
      *
      * @param polygon the polygon to test
      * @return true if polygon is inside this one.
      */
     bool envelopes(const ConvexPolygon &polygon) const;

     /**
      * Check if this polygon overlaps with the given polygon.
      *
      * The given polygon may be completely inside this one, or
      * it may intersect this one.
      * @param polygon the polygon to test
      * @return true if polygons overlap
      */
     bool overlaps(const ConvexPolygon &polygon) const;

     /**
      * Apply a boolean difference operation to this polygon.
      *
      * The newly generated polygons will be added to the provided list.
      * If this polygon is enveloped totally by the hole, no polygons will
      * be added.
      * 
      * @param hole the polygon that defines the "hole" to make.
      * @param list the vector to which the new polygons should be added
      */
     void booleanDifference(const ConvexPolygon &hole, std::vector<ConvexPolygon> &list) const;

     /**
      * Get the number of vertices in this polygon.
      *
      * If this is not a valid polygon, 0 will be returned.
      * @return vertex count or 0 if not initialized
      */
     int vertexCount() const { return m_points.size(); }

     /**
      * The vertex vector.
      *
      * @return reference to the vertex vector.
      */
     const Points &vertices() const { return m_points; }

     /**
      * Get the vertex at the given index.
      *
      * Index must be in range [-1..vertexCount()]
      * For convenience, the last index returns the same point as 0
      * and -1 will return the last point.
      *
      * @param i vertex index
      * @return vertex
      */
     const Point &vertex(int i) const {
         const int s = m_points.size();
         assert(i >= -1 && i <= s);
         if(i<0)
             return m_points[s-1];
         if(i<s)
             return m_points[i];
         return m_points[0];
    }

     /**
      * Edge normals
      *
      * @return reference to edge normal vector
      */
     const std::vector<glm::vec2> &normals() const { return m_normals; }

     /**
      * Get the polygon bounding box
      */
     const BRect &bounds() const { return m_bounds; }

     /**
      * Triangulate this polygon.
      *
      * @param points the vector to which the vertices will be added
      */
     void toTriangles(Points &points) const;
 
 private:
     Points m_points;
     std::vector<glm::vec2> m_normals;
     BRect m_bounds;
 };

}

#endif
 
