/*************************************************************************
This file is part of Logical Vision 2.

Logical Vision 2 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Logical Vision 2 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
************************************************************************/
/* Drawing library for prolog
 * ============================
 * Version: 2.0
 * Author: Wang-Zhou Dai <dai.wzero@gmail.com>
 */

#include "draw.hpp"
#include "sampler.hpp"
#include "utils.hpp"
#include "memread.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <SWI-cpp.h>
#include <SWI-Prolog.h>

/* get color Lab value from prolog term */
Scalar term2color(PlTerm C) {
    char *p4 = (char*) C;
    Scalar color; // color
    // color
    if (p4[0] == 'r' || p4[0] == 'R')
        color = RED;
    else if (p4[0] == 'g' || p4[0] == 'G')
        color = GREEN;
    else if (p4[0] == 'b' || p4[0] == 'B')
        color = BLUE;
    else if (p4[0] == 'y' || p4[0] == 'Y')
        color = YELLOW;
    else if (p4[0] == 'w' || p4[0] == 'W')
        color = WHITE;
    else
        color = BLACK;
    return color;
}

/* draw_line_seg(+IMGSEQ, +START, +END, +COLOR)
 * @IMGSEQ: address of image sequence
 * @START = [SX, SY, SZ]: starting point
 * @END = [EX, EY, EZ]: ending point
 * @COLOR = <r(ed)/g(reen)/b(lue)/y(ellow)>: line color
 */
PREDICATE(draw_line_seg, 4) {
    // parsing arguments
    char *p1 = (char*) A1;
    const string add_seq(p1); // address
    vector<Mat> *seq = str2ptr<vector<Mat>>(add_seq);
    vector<int> start_v = list2vec<int>(A2, 3);
    vector<int> end_v = list2vec<int>(A3, 3);
    Scalar start(start_v[0], start_v[1], start_v[2]); // start point
    Scalar end(end_v[0], end_v[1], end_v[2]); // end point
    Scalar color = term2color(A4); // color
    // get all line points
    Scalar bound((*seq)[0].cols, (*seq)[0].rows, seq->size());
    vector<Scalar> line_points = get_line_seg_points(start, end, bound);
    // draw
    for (auto it = line_points.begin(); it != line_points.end(); ++it) {
        Scalar pt = (Scalar) *it;
        cv_draw_point((*seq)[pt[2]], Point(pt[0], pt[1]), color);
    }
    return TRUE;
}
    
/* draw_line_seg_2d(+IMG, +START, +END, +COLOR)
 * @IMG: address of image
 * @START = [SX, SY, SZ]: starting point
 * @END = [EX, EY, EZ]: ending point
 * @COLOR = <r(ed)/g(reen)/b(lue)/y(ellow)>: line color
 */
PREDICATE(draw_line_seg_2d, 4) {
    // parsing arguments
    char *p1 = (char*) A1;
    const string add_img(p1); // address
    Mat *img = str2ptr<Mat>(add_img);
    vector<int> start_v = list2vec<int>(A2, 2);
    vector<int> end_v = list2vec<int>(A3, 2);
    Scalar start(start_v[0], start_v[1], -1); // start point
    Scalar end(end_v[0], end_v[1], -1); // end point
    Scalar color = term2color(A4); // color    
    // draw
    cv_draw_line(*img,
                 Point(start[0], start[1]),
                 Point(end[0], end[1]),
                 color);
    return TRUE;
}

/* draw_rect(+SEQ, +CENTER, +RADIUS, +COLOR)
 * @SEQ: img sequence
 * @CENTER: rectangle center [X, Y, Z]
 * @RADIUS: rectangle radius [RX, RY, RZ]
 * @COLOR: rectangle color
 */
PREDICATE(draw_rect, 4) {
    // parsing arguments
    char *p1 = (char*) A1;
    const string add_seq(p1); // address
    vector<Mat> *seq = str2ptr<vector<Mat>>(add_seq);
    vector<int> center = list2vec<int>(A2, 3);
    vector<int> radius = list2vec<int>(A3, 3);
    Scalar c(center[0], center[1], 0);
    Scalar r(radius[0], radius[1], 0);
    Scalar color = term2color(A4); // color
    // get start and end frame
    int T = seq->size() - 1;
    int f_start = (center[2] - radius[2] >= 0) ? (center[2] - radius[2]) : 0;
    int f_end = (center[2] + radius[2] <= T) ? (center[2] + radius[2]) : T;
    // draw
    for (int i = f_start; i <= f_end; i++) {
        cv_draw_rect((*seq)[i], c, r, color);
    }
    return TRUE;
}

/* draw_rect_2d(+IMG, +CENTER, +RADIUS, +COLOR)
 * @SEQ: img sequence
 * @CENTER: rectangle center [X, Y, Z]
 * @RADIUS: rectangle radius [RX, RY, RZ]
 * @COLOR: rectangle color 
 */
PREDICATE(draw_rect_2d, 4) {
    // parsing arguments
    char *p1 = (char*) A1;
    const string add_img(p1); // address
    Mat *img = str2ptr<Mat>(add_img);
    vector<int> center = list2vec<int>(A2, 2);
    vector<int> radius = list2vec<int>(A3, 2);
    Scalar c(center[0], center[1], 0);
    Scalar r(radius[0], radius[1], 0);
    Scalar color = term2color(A4); // color
    // draw
    cv_draw_rect(*img, c, r, color);
    return TRUE;
}

/* draw_points(+SEQ, +PTS, +COLOR)
 * @SEQ: address of image sequence
 * @PTS = [[P1X, P1Y, P1Z], ...]: list of points
 * @COLOR = <r(ed)/g(reen)/b(lue)/y(ellow)>: line color
 */
PREDICATE(draw_points, 3) {
    // parsing arguments
    char *p1 = (char*) A1;
    const string add_seq(p1); // address
    vector<Mat> *seq = str2ptr<vector<Mat>>(add_seq);
    vector<Scalar> pts = point_list2vec(A2);
    if (pts.empty())
        return TRUE;
    else {
        Scalar color = term2color(A3); // color    
        // draw
        for (auto it = pts.begin(); it != pts.end(); ++it) {
            Scalar pt = (Scalar) *it;
            cv_draw_point((*seq)[pt[2]], Point(pt[0], pt[1]), color);
        }
        return TRUE;
    }
}

/* draw_points_2d(+IMG, +PTS, +COLOR)
 * @IMG: address of image
 * @PTS = [[P1X, P1Y, P1Z], ...]: list of points
 * @COLOR = <r(ed)/g(reen)/b(lue)/y(ellow)>: line color
 */
PREDICATE(draw_points_2d, 3) {
    // parsing arguments
    char *p1 = (char*) A1;
    const string add_img(p1); // address
    Mat *img = str2ptr<Mat>(add_img);
    vector<Scalar> pts = point_list2vec(A2);
    if (pts.empty())
        return TRUE;
    else {
        Scalar color = term2color(A3); // color    
        // draw
        for (auto it = pts.begin(); it != pts.end(); ++it) {
            Scalar pt = (Scalar) *it;
            cv_draw_point(*img, Point(pt[0], pt[1]), color);
        }
        return TRUE;
    }
}
