/*
 * SparkFun_2DRayTracing.h
 *
 * Pure-C port for FRDM-KL25Z / MCUXpresso
 *
 * Original author:
 * Owen Lyke
 *
 * Changes for KL25Z:
 *  - Removed Arduino.h
 *  - Removed C++ classes
 *  - Removed constructors
 *  - Removed inheritance / virtual functions
 *  - Replaced max() / min() with C helper functions
 *  - Converted class methods into static inline C functions
 */

#ifndef SF_2DRAYTRACING_H_
#define SF_2DRAYTRACING_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <math.h>


/* ==========================================================================
 * Configurable data types
 * ========================================================================== */

typedef uint16_t sf2drt_max_qty_t;
typedef double   sf2drt_extent_t;
typedef double   sf2drt_diff_t;
typedef double   sf2drt_rotation_t;


/* ==========================================================================
 * Basic structures
 * ========================================================================== */

typedef struct
{
    sf2drt_extent_t x;
    sf2drt_extent_t y;

} sf2drt_coordinate_t;


typedef struct
{
    sf2drt_extent_t minx;
    sf2drt_extent_t miny;
    sf2drt_extent_t maxx;
    sf2drt_extent_t maxy;

} sf2drt_bounds_t;


typedef enum
{
    sf2drt_orientation_positive = 0x00,
    sf2drt_orientation_colinear,
    sf2drt_orientation_negative

} sf2drt_orientation_t;


/* ==========================================================================
 * Local utility functions
 * ========================================================================== */

static inline sf2drt_extent_t sf2drt_max(
    sf2drt_extent_t a,
    sf2drt_extent_t b)
{
    return (a > b) ? a : b;
}


static inline sf2drt_extent_t sf2drt_min(
    sf2drt_extent_t a,
    sf2drt_extent_t b)
{
    return (a < b) ? a : b;
}


/* ==========================================================================
 * Triplet
 *
 * A triplet is an ordered set of three coordinates.
 * ========================================================================== */

typedef struct
{
    sf2drt_coordinate_t p1;
    sf2drt_coordinate_t p2;
    sf2drt_coordinate_t p3;

} sf2drt_triplet_t;


/*
 * Initialize a triplet.
 */
static inline void sf2drt_triplet_init(
    sf2drt_triplet_t *triplet,
    sf2drt_coordinate_t p1,
    sf2drt_coordinate_t p2,
    sf2drt_coordinate_t p3)
{
    if (triplet == NULL)
    {
        return;
    }

    triplet->p1 = p1;
    triplet->p2 = p2;
    triplet->p3 = p3;
}


/*
 * Determine orientation of the three points.
 */
static inline sf2drt_orientation_t sf2drt_triplet_orientation(
    const sf2drt_triplet_t *triplet)
{
    double cross;

    if (triplet == NULL)
    {
        return sf2drt_orientation_colinear;
    }

    cross =
        ((double)triplet->p2.y - (double)triplet->p1.y) *
        ((double)triplet->p3.x - (double)triplet->p2.x)
        -
        ((double)triplet->p2.x - (double)triplet->p1.x) *
        ((double)triplet->p3.y - (double)triplet->p2.y);

    if (cross == 0.0)
    {
        return sf2drt_orientation_colinear;
    }

    if (cross > 0.0)
    {
        return sf2drt_orientation_negative;
    }

    return sf2drt_orientation_positive;
}


/*
 * Determine whether p3 lies inside the bounding box formed by p1 and p2.
 */
static inline bool sf2drt_triplet_p3contained(
    const sf2drt_triplet_t *triplet)
{
    if (triplet == NULL)
    {
        return false;
    }

    if ((triplet->p3.x <=
         sf2drt_max(triplet->p1.x, triplet->p2.x)) &&

        (triplet->p3.x >=
         sf2drt_min(triplet->p1.x, triplet->p2.x)) &&

        (triplet->p3.y <=
         sf2drt_max(triplet->p1.y, triplet->p2.y)) &&

        (triplet->p3.y >=
         sf2drt_min(triplet->p1.y, triplet->p2.y)))
    {
        return true;
    }

    return false;
}


/* ==========================================================================
 * Line
 * ========================================================================== */

typedef struct
{
    sf2drt_coordinate_t p1;
    sf2drt_coordinate_t p2;

} sf2drt_line_t;


/*
 * Initialize a line.
 */
static inline void sf2drt_line_init(
    sf2drt_line_t *line,
    sf2drt_coordinate_t p1,
    sf2drt_coordinate_t p2)
{
    if (line == NULL)
    {
        return;
    }

    line->p1 = p1;
    line->p2 = p2;
}


static inline void sf2drt_line_setP1(
    sf2drt_line_t *line,
    sf2drt_coordinate_t p1)
{
    if (line == NULL)
    {
        return;
    }

    line->p1 = p1;
}


static inline void sf2drt_line_setP2(
    sf2drt_line_t *line,
    sf2drt_coordinate_t p2)
{
    if (line == NULL)
    {
        return;
    }

    line->p2 = p2;
}


/*
 * Determine whether two line segments intersect.
 */
static inline bool sf2drt_line_intersects(
    const sf2drt_line_t *line,
    const sf2drt_line_t *other)
{
    sf2drt_triplet_t t11;
    sf2drt_triplet_t t12;
    sf2drt_triplet_t t21;
    sf2drt_triplet_t t22;

    sf2drt_orientation_t o11;
    sf2drt_orientation_t o12;
    sf2drt_orientation_t o21;
    sf2drt_orientation_t o22;

    if ((line == NULL) || (other == NULL))
    {
        return false;
    }

    sf2drt_triplet_init(
        &t11,
        line->p1,
        line->p2,
        other->p1);

    sf2drt_triplet_init(
        &t12,
        line->p1,
        line->p2,
        other->p2);

    sf2drt_triplet_init(
        &t21,
        other->p1,
        other->p2,
        line->p1);

    sf2drt_triplet_init(
        &t22,
        other->p1,
        other->p2,
        line->p2);


    o11 = sf2drt_triplet_orientation(&t11);
    o12 = sf2drt_triplet_orientation(&t12);
    o21 = sf2drt_triplet_orientation(&t21);
    o22 = sf2drt_triplet_orientation(&t22);


    /*
     * General intersection case.
     */
    if ((o11 != o12) && (o21 != o22))
    {
        return true;
    }


    /*
     * Special colinear cases.
     */
    if ((o11 == sf2drt_orientation_colinear) &&
        sf2drt_triplet_p3contained(&t11))
    {
        return true;
    }


    if ((o21 == sf2drt_orientation_colinear) &&
        sf2drt_triplet_p3contained(&t21))
    {
        return true;
    }


    if ((o12 == sf2drt_orientation_colinear) &&
        sf2drt_triplet_p3contained(&t12))
    {
        return true;
    }


    if ((o22 == sf2drt_orientation_colinear) &&
        sf2drt_triplet_p3contained(&t22))
    {
        return true;
    }


    return false;
}


/* ==========================================================================
 * Polygon
 *
 * The original C++ sf2drt_area base class is unnecessary in C.
 *
 * A polygon simply contains:
 *
 *      number of sides
 *      pointer to vertices
 *      rotation value
 * ========================================================================== */

typedef struct
{
    sf2drt_max_qty_t num_sides;

    sf2drt_coordinate_t *p_points;

    sf2drt_rotation_t rotation;

} sf2drt_polygon_t;


/*
 * Initialize an empty polygon.
 */
static inline void sf2drt_polygon_init_empty(
    sf2drt_polygon_t *polygon)
{
    if (polygon == NULL)
    {
        return;
    }

    polygon->p_points = NULL;
    polygon->num_sides = 0U;
    polygon->rotation = 0.0;
}


/*
 * Initialize polygon using an existing point array.
 *
 * The point array must remain valid while the polygon is in use.
 */
static inline void sf2drt_polygon_init(
    sf2drt_polygon_t *polygon,
    sf2drt_max_qty_t num_sides,
    sf2drt_coordinate_t *p_points)
{
    if (polygon == NULL)
    {
        return;
    }

    polygon->p_points = p_points;
    polygon->num_sides = num_sides;
    polygon->rotation = 0.0;
}


/*
 * Return polygon bounds.
 */
static inline sf2drt_bounds_t sf2drt_polygon_getBounds(
    const sf2drt_polygon_t *polygon)
{
    sf2drt_bounds_t bounds =
    {
        0.0,
        0.0,
        0.0,
        0.0
    };

    sf2drt_max_qty_t i;


    if ((polygon == NULL) ||
        (polygon->p_points == NULL) ||
        (polygon->num_sides == 0U))
    {
        return bounds;
    }


    bounds.maxx = polygon->p_points[0].x;
    bounds.minx = polygon->p_points[0].x;

    bounds.maxy = polygon->p_points[0].y;
    bounds.miny = polygon->p_points[0].y;


    for (i = 1U; i < polygon->num_sides; i++)
    {
        bounds.maxx =
            sf2drt_max(
                bounds.maxx,
                polygon->p_points[i].x);

        bounds.minx =
            sf2drt_min(
                bounds.minx,
                polygon->p_points[i].x);

        bounds.maxy =
            sf2drt_max(
                bounds.maxy,
                polygon->p_points[i].y);

        bounds.miny =
            sf2drt_min(
                bounds.miny,
                polygon->p_points[i].y);
    }


    return bounds;
}


/*
 * Determine whether point p is inside polygon.
 *
 * Uses the same ray-intersection idea as the original SparkFun code.
 */
static inline bool sf2drt_polygon_contains(
    const sf2drt_polygon_t *polygon,
    sf2drt_coordinate_t p)
{
    sf2drt_bounds_t bounds;
    sf2drt_coordinate_t p2;

    sf2drt_line_t bisector;
    sf2drt_line_t segment;

    sf2drt_max_qty_t i;

    uint8_t inside = 0U;


    if ((polygon == NULL) ||
        (polygon->p_points == NULL) ||
        (polygon->num_sides < 3U))
    {
        return false;
    }


    bounds = sf2drt_polygon_getBounds(polygon);


    if ((p.x < bounds.minx) ||
        (p.x > bounds.maxx) ||
        (p.y < bounds.miny) ||
        (p.y > bounds.maxy))
    {
        return false;
    }


    /*
     * Create a horizontal ray from p toward the right.
     *
     * Move one unit beyond the polygon's maximum X coordinate so the
     * ray does not become zero-length when p.x == bounds.maxx.
     */
    p2 = p;
    p2.x = bounds.maxx + 1.0;


    sf2drt_line_init(
        &bisector,
        p,
        p2);


    for (i = 0U;
         i < (sf2drt_max_qty_t)(polygon->num_sides - 1U);
         i++)
    {
        sf2drt_line_init(
            &segment,
            polygon->p_points[i],
            polygon->p_points[i + 1U]);


        if (sf2drt_line_intersects(
                &bisector,
                &segment))
        {
            inside ^= 0x01U;
        }
    }


    /*
     * Last polygon segment:
     *
     * last point -> first point
     */
    sf2drt_line_init(
        &segment,
        polygon->p_points[polygon->num_sides - 1U],
        polygon->p_points[0]);


    if (sf2drt_line_intersects(
            &bisector,
            &segment))
    {
        inside ^= 0x01U;
    }


    return ((inside & 0x01U) != 0U);
}


/*
 * Move all polygon vertices.
 */
static inline void sf2drt_polygon_displace(
    sf2drt_polygon_t *polygon,
    sf2drt_diff_t dx,
    sf2drt_diff_t dy)
{
    sf2drt_max_qty_t i;


    if ((polygon == NULL) ||
        (polygon->p_points == NULL))
    {
        return;
    }


    for (i = 0U;
         i < polygon->num_sides;
         i++)
    {
        polygon->p_points[i].x += dx;
        polygon->p_points[i].y += dy;
    }
}


/*
 * Set one polygon point.
 *
 * Returns true if successful.
 */
static inline bool sf2drt_polygon_setPn(
    sf2drt_polygon_t *polygon,
    sf2drt_max_qty_t n,
    sf2drt_coordinate_t p)
{
    if ((polygon == NULL) ||
        (polygon->p_points == NULL) ||
        (n >= polygon->num_sides))
    {
        return false;
    }


    polygon->p_points[n] = p;

    return true;
}


/*
 * Get one polygon point.
 *
 * Returns true if successful.
 */
static inline bool sf2drt_polygon_getPn(
    const sf2drt_polygon_t *polygon,
    sf2drt_max_qty_t n,
    sf2drt_coordinate_t *p)
{
    if ((polygon == NULL) ||
        (polygon->p_points == NULL) ||
        (p == NULL) ||
        (n >= polygon->num_sides))
    {
        return false;
    }


    *p = polygon->p_points[n];

    return true;
}


/*
 * Record polygon rotation.
 *
 * The original SparkFun implementation only updated the stored
 * rotation value. It did not actually rotate the vertices.
 *
 * This behavior is preserved here.
 */
static inline void sf2drt_polygon_rotate(
    sf2drt_polygon_t *polygon,
    sf2drt_rotation_t alpha,
    sf2drt_coordinate_t center)
{
    (void)center;


    if (polygon == NULL)
    {
        return;
    }


    polygon->rotation += alpha;
}


/*
 * Return average location of all polygon vertices.
 *
 * This is called "center of mass" by the original library,
 * although technically it is the average vertex position.
 */
static inline sf2drt_coordinate_t sf2drt_polygon_getCOM(
    const sf2drt_polygon_t *polygon)
{
    sf2drt_coordinate_t result =
    {
        0.0,
        0.0
    };

    double avgX = 0.0;
    double avgY = 0.0;

    sf2drt_max_qty_t i;


    if ((polygon == NULL) ||
        (polygon->p_points == NULL) ||
        (polygon->num_sides == 0U))
    {
        return result;
    }


    for (i = 0U;
         i < polygon->num_sides;
         i++)
    {
        avgX +=
            (double)polygon->p_points[i].x;

        avgY +=
            (double)polygon->p_points[i].y;
    }


    avgX /= (double)polygon->num_sides;
    avgY /= (double)polygon->num_sides;


    result.x =
        (sf2drt_extent_t)round(avgX);

    result.y =
        (sf2drt_extent_t)round(avgY);


    return result;
}


/* ==========================================================================
 * Equilateral triangle
 *
 * The original SparkFun implementation did not actually calculate
 * the triangle geometry. That behavior is preserved.
 * ========================================================================== */

typedef struct
{
    sf2drt_polygon_t polygon;

    sf2drt_coordinate_t points[3];

} sf2drt_equilateral_center_tip_t;


static inline void sf2drt_equilateral_center_tip_init(
    sf2drt_equilateral_center_tip_t *triangle,
    sf2drt_coordinate_t center,
    sf2drt_coordinate_t tip)
{
    if (triangle == NULL)
    {
        return;
    }


    sf2drt_polygon_init(
        &triangle->polygon,
        3U,
        triangle->points);


    /*
     * Original library left this calculation unfinished.
     *
     * Prevent compiler warnings for now.
     */
    (void)center;
    (void)tip;
}


/* ==========================================================================
 * Rectangle defined by center and corner
 * ========================================================================== */

typedef struct
{
    sf2drt_polygon_t polygon;

    sf2drt_coordinate_t points[4];

} sf2drt_rect_center_corner_t;


static inline void sf2drt_rect_center_corner_init(
    sf2drt_rect_center_corner_t *rect,
    sf2drt_coordinate_t center,
    sf2drt_coordinate_t corner)
{
    double x;
    double y;


    if (rect == NULL)
    {
        return;
    }


    sf2drt_polygon_init(
        &rect->polygon,
        4U,
        rect->points);


    rect->points[0] = corner;


    x =
        (double)(2.0 * center.x - corner.x);

    if (x < 0.0)
    {
        x = 0.0;
    }


    rect->points[1].x =
        (sf2drt_extent_t)x;

    rect->points[1].y =
        corner.y;


    y =
        (double)(2.0 * center.y - corner.y);

    if (y < 0.0)
    {
        y = 0.0;
    }


    rect->points[2].x =
        (sf2drt_extent_t)x;

    rect->points[2].y =
        (sf2drt_extent_t)y;


    rect->points[3].x =
        corner.x;

    rect->points[3].y =
        (sf2drt_extent_t)y;
}


/* ==========================================================================
 * Rectangle defined by two opposite corners
 * ========================================================================== */

typedef struct
{
    sf2drt_polygon_t polygon;

    sf2drt_coordinate_t points[4];

} sf2drt_rect_2corner_t;


static inline void sf2drt_rect_2corner_init(
    sf2drt_rect_2corner_t *rect,
    sf2drt_coordinate_t corner1,
    sf2drt_coordinate_t corner2)
{
    if (rect == NULL)
    {
        return;
    }


    sf2drt_polygon_init(
        &rect->polygon,
        4U,
        rect->points);


    rect->points[0] =
        corner1;


    rect->points[1].x =
        corner2.x;

    rect->points[1].y =
        corner1.y;


    rect->points[2] =
        corner2;


    rect->points[3].x =
        corner1.x;

    rect->points[3].y =
        corner2.y;
}


#endif /* SF_2DRAYTRACING_H_ */
