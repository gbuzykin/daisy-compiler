/* Parsegen autogenerated analyzer file - do not edit! */
/* clang-format off */

static int action_idx[213] = {
    0, 2, 20, 2, 2, 2, 2, 2, 62, 64, 66, 68, 72, 76, 80, 82, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    84, 120, 154, 170, 186, 210, 234, 246, 258, 290, 320, 2, 362, 402, 418, 434, 436, 444, 452, 454, 482, 484, 486, 488,
    530, 532, 534, 536, 550, 552, 554, 558, 560, 564, 568, 572, 578, 582, 584, 588, 592, 596, 598, 602, 606, 608, 612,
    614, 618, 620, 624, 626, 630, 602, 632, 634, 636, 638, 640, 642, 682, 684, 726, 730, 734, 736, 738, 740, 742, 744,
    748, 748, 748, 748, 778, 748, 780, 784, 786, 788, 790, 564, 792, 796, 828, 796, 832, 828, 872, 874, 828, 878, 748,
    748, 748, 748, 748, 748, 748, 748, 748, 748, 748, 748, 748, 748, 748, 748, 748, 748, 748, 880, 916, 950, 966, 982,
    1006, 1030, 1042, 1054, 1086, 1116, 748, 1158, 1198, 1214, 1230, 1232, 1240, 1248, 1250, 1278, 1280, 828, 1282,
    1286, 1292, 1294, 1296, 1298, 1304, 1306, 1310, 602, 1314, 748, 1316, 792, 1356, 1358, 1360, 1366, 1306, 1368, 748,
    1372, 564, 1412, 1414, 1416, 1418, 1420, 1422, 1424, 1466, 1468, 1470, 1472, 1474, 1478, 1480, 1486, 1488, 1492,
    1494, 1496, 1510, 1512
};

static int action_list[1514] = {
    -1, 12, 33, 7, 40, 9, 43, 11, 45, 13, 126, 15, 261, 17, 262, 19, 265, 21, -1, -1, 37, 33, 38, 35, 42, 37, 43, 39,
    45, 41, 47, 43, 60, 45, 62, 47, 63, 49, 94, 51, 124, 53, 260, 123, 269, 55, 270, 57, 271, 59, 272, 61, 273, 63, 274,
    65, 275, 67, 276, 69, -1, -1, -1, 690, -1, 696, -1, 678, 40, 25, -1, -1, 265, 27, -1, -1, 41, 29, -1, -1, -1, 684,
    -1, 588, 37, 33, 38, 35, 42, 37, 43, 39, 45, 41, 47, 43, 60, 45, 62, 47, 94, 51, 124, 53, 269, 55, 270, 57, 271, 59,
    272, 61, 273, 63, 274, 65, 275, 67, -1, 660, 37, 33, 38, 35, 42, 37, 43, 39, 45, 41, 47, 43, 60, 45, 62, 47, 94, 51,
    124, 53, 269, 55, 270, 57, 271, 59, 272, 61, 273, 63, 274, 65, -1, 654, 37, 33, 42, 37, 43, 39, 45, 41, 47, 43, 269,
    55, 270, 57, -1, 642, 37, 33, 42, 37, 43, 39, 45, 41, 47, 43, 269, 55, 270, 57, -1, 630, 37, 33, 42, 37, 43, 39, 45,
    41, 47, 43, 60, 45, 62, 47, 269, 55, 270, 57, 273, 63, 274, 65, -1, 618, 37, 33, 42, 37, 43, 39, 45, 41, 47, 43, 60,
    45, 62, 47, 269, 55, 270, 57, 273, 63, 274, 65, -1, 612, 37, 33, 42, 37, 43, 39, 45, 41, 47, 43, -1, 582, 37, 33,
    42, 37, 43, 39, 45, 41, 47, 43, -1, 576, 37, 33, 38, 35, 42, 37, 43, 39, 45, 41, 47, 43, 60, 45, 62, 47, 94, 51,
    269, 55, 270, 57, 271, 59, 272, 61, 273, 63, 274, 65, -1, 600, 37, 33, 38, 35, 42, 37, 43, 39, 45, 41, 47, 43, 60,
    45, 62, 47, 269, 55, 270, 57, 271, 59, 272, 61, 273, 63, 274, 65, -1, 606, 37, 33, 38, 35, 42, 37, 43, 39, 45, 41,
    47, 43, 58, 93, 60, 45, 62, 47, 63, 49, 94, 51, 124, 53, 269, 55, 270, 57, 271, 59, 272, 61, 273, 63, 274, 65, 275,
    67, 276, 69, -1, -1, 37, 33, 38, 35, 42, 37, 43, 39, 45, 41, 47, 43, 60, 45, 62, 47, 63, 49, 94, 51, 124, 53, 269,
    55, 270, 57, 271, 59, 272, 61, 273, 63, 274, 65, 275, 67, 276, 69, -1, 666, 37, 33, 42, 37, 43, 39, 45, 41, 47, 43,
    269, 55, 270, 57, -1, 636, 37, 33, 42, 37, 43, 39, 45, 41, 47, 43, 269, 55, 270, 57, -1, 624, -1, 564, 37, 33, 42,
    37, 47, 43, -1, 552, 37, 33, 42, 37, 47, 43, -1, 546, -1, 558, 37, 33, 42, 37, 43, 39, 45, 41, 47, 43, 60, 45, 62,
    47, 269, 55, 270, 57, 271, 59, 272, 61, 273, 63, 274, 65, -1, 594, -1, 570, -1, 534, -1, 540, 37, 33, 38, 35, 41,
    119, 42, 37, 43, 39, 45, 41, 47, 43, 60, 45, 62, 47, 63, 49, 94, 51, 124, 53, 269, 55, 270, 57, 271, 59, 272, 61,
    273, 63, 274, 65, 275, 67, 276, 69, -1, -1, -1, 672, -1, 648, -1, -1, 59, 129, 258, 131, 259, 133, 288, 135, 289,
    137, 291, 139, -1, -1, -1, 6, -1, 54, 59, 425, -1, -1, -1, 0, 265, 415, -1, -1, 265, 375, -1, -1, 265, 143, -1, -1,
    59, 183, 123, 185, -1, -1, 40, 145, -1, -1, -1, 114, 265, 155, -1, 144, 41, 173, -1, -1, 44, 169, -1, 138, -1, 150,
    58, 157, -1, -1, 267, 163, -1, 474, -1, 162, 265, 165, -1, -1, -1, 480, 267, 167, -1, 108, -1, 486, 265, 155, -1,
    -1, -1, 156, 287, 177, -1, 132, -1, 120, -1, 126, -1, 42, -1, 36, -1, 168, -1, 240, 33, 211, 40, 213, 43, 215, 45,
    217, 59, 219, 123, 185, 126, 221, 258, 223, 261, 225, 262, 227, 263, 229, 264, 231, 265, 474, 267, 163, 289, 233,
    290, 235, 292, 237, 294, 239, 295, 241, -1, 258, -1, 264, 37, 255, 38, 257, 42, 259, 43, 261, 45, 263, 47, 265, 59,
    409, 60, 267, 62, 269, 63, 271, 94, 273, 124, 275, 269, 277, 270, 279, 271, 281, 272, 283, 273, 285, 274, 287, 275,
    289, 276, 291, -1, 246, 265, 407, -1, -1, 125, 405, -1, -1, -1, 270, -1, 276, -1, 234, -1, 468, -1, 462, 264, 403,
    -1, 522, 33, 211, 40, 213, 43, 215, 45, 217, 123, 185, 126, 221, 261, 225, 262, 227, 263, 229, 264, 231, 267, 163,
    292, 237, 294, 239, 295, 241, -1, 474, -1, 306, 59, 389, -1, 252, -1, 504, -1, 510, -1, 516, -1, 492, 265, 353, -1,
    -1, 33, 211, 40, 213, 43, 215, 45, 217, 123, 185, 126, 221, 258, 247, 261, 225, 262, 227, 263, 229, 264, 231, 267,
    163, 292, 237, 294, 239, 295, 241, -1, 474, 123, 185, -1, -1, 37, 255, 38, 257, 42, 259, 43, 261, 45, 263, 47, 265,
    60, 267, 62, 269, 63, 271, 94, 273, 124, 275, 269, 277, 270, 279, 271, 281, 272, 283, 273, 285, 274, 287, 275, 289,
    276, 291, -1, 192, -1, 198, 293, 251, -1, 222, -1, 228, 37, 255, 38, 257, 42, 259, 43, 261, 45, 263, 47, 265, 60,
    267, 62, 269, 94, 273, 124, 275, 269, 277, 270, 279, 271, 281, 272, 283, 273, 285, 274, 287, 275, 289, -1, 438, 37,
    255, 38, 257, 42, 259, 43, 261, 45, 263, 47, 265, 60, 267, 62, 269, 94, 273, 124, 275, 269, 277, 270, 279, 271, 281,
    272, 283, 273, 285, 274, 287, -1, 432, 37, 255, 42, 259, 43, 261, 45, 263, 47, 265, 269, 277, 270, 279, -1, 420, 37,
    255, 42, 259, 43, 261, 45, 263, 47, 265, 269, 277, 270, 279, -1, 408, 37, 255, 42, 259, 43, 261, 45, 263, 47, 265,
    60, 267, 62, 269, 269, 277, 270, 279, 273, 285, 274, 287, -1, 396, 37, 255, 42, 259, 43, 261, 45, 263, 47, 265, 60,
    267, 62, 269, 269, 277, 270, 279, 273, 285, 274, 287, -1, 390, 37, 255, 42, 259, 43, 261, 45, 263, 47, 265, -1, 360,
    37, 255, 42, 259, 43, 261, 45, 263, 47, 265, -1, 354, 37, 255, 38, 257, 42, 259, 43, 261, 45, 263, 47, 265, 60, 267,
    62, 269, 94, 273, 269, 277, 270, 279, 271, 281, 272, 283, 273, 285, 274, 287, -1, 378, 37, 255, 38, 257, 42, 259,
    43, 261, 45, 263, 47, 265, 60, 267, 62, 269, 269, 277, 270, 279, 271, 281, 272, 283, 273, 285, 274, 287, -1, 384,
    37, 255, 38, 257, 42, 259, 43, 261, 45, 263, 47, 265, 58, 315, 60, 267, 62, 269, 63, 271, 94, 273, 124, 275, 269,
    277, 270, 279, 271, 281, 272, 283, 273, 285, 274, 287, 275, 289, 276, 291, -1, -1, 37, 255, 38, 257, 42, 259, 43,
    261, 45, 263, 47, 265, 60, 267, 62, 269, 63, 271, 94, 273, 124, 275, 269, 277, 270, 279, 271, 281, 272, 283, 273,
    285, 274, 287, 275, 289, 276, 291, -1, 444, 37, 255, 42, 259, 43, 261, 45, 263, 47, 265, 269, 277, 270, 279, -1,
    414, 37, 255, 42, 259, 43, 261, 45, 263, 47, 265, 269, 277, 270, 279, -1, 402, -1, 342, 37, 255, 42, 259, 47, 265,
    -1, 330, 37, 255, 42, 259, 47, 265, -1, 324, -1, 336, 37, 255, 42, 259, 43, 261, 45, 263, 47, 265, 60, 267, 62, 269,
    269, 277, 270, 279, 271, 281, 272, 283, 273, 285, 274, 287, -1, 372, -1, 348, -1, 216, 293, 341, -1, 180, 123, 185,
    292, 237, -1, -1, -1, 210, -1, 204, -1, 186, 44, 365, 59, 367, -1, -1, -1, 78, 58, 357, -1, 102, 61, 361, -1, -1,
    -1, 96, 37, 255, 38, 257, 42, 259, 43, 261, 45, 263, 47, 265, 60, 267, 62, 269, 63, 271, 94, 273, 124, 275, 269,
    277, 270, 279, 271, 281, 272, 283, 273, 285, 274, 287, 275, 289, 276, 291, -1, 90, -1, 294, -1, 84, 44, 383, 59,
    385, -1, -1, -1, 60, 61, 379, -1, -1, 37, 255, 38, 257, 42, 259, 43, 261, 45, 263, 47, 265, 60, 267, 62, 269, 63,
    271, 94, 273, 124, 275, 269, 277, 270, 279, 271, 281, 272, 283, 273, 285, 274, 287, 275, 289, 276, 291, -1, 72, -1,
    288, -1, 66, -1, 300, -1, 366, -1, 312, -1, 318, 37, 255, 38, 257, 41, 399, 42, 259, 43, 261, 45, 263, 47, 265, 60,
    267, 62, 269, 63, 271, 94, 273, 124, 275, 269, 277, 270, 279, 271, 281, 272, 283, 273, 285, 274, 287, 275, 289, 276,
    291, -1, -1, -1, 450, -1, 426, -1, 498, -1, 174, 267, 167, -1, 456, -1, 282, 44, 383, 59, 413, -1, -1, -1, 30, 123,
    417, -1, -1, -1, 18, -1, 12, 59, 129, 125, 423, 258, 131, 288, 135, 289, 137, 291, 139, -1, -1, -1, 24, -1, 48
};

static int reduce_info[351] = {
    2, 0, 0, 2, 2, 0, 0, 2, 0, 0, 8, 1, 6, 6, 2, 3, 6, 0, 3, 6, 47, 3, 6, 48, 2, 6, 0, 1, 6, 0, 1, 10, 0, 3, 10, 0, 4,
    30, 36, 1, 96, 0, 3, 96, 0, 4, 98, 37, 2, 34, 0, 0, 34, 0, 2, 102, 0, 0, 120, 46, 6, 14, 0, 2, 118, 0, 0, 118, 0, 1,
    116, 0, 0, 116, 0, 1, 122, 0, 3, 122, 0, 3, 124, 49, 0, 132, 38, 5, 16, 2, 3, 134, 41, 5, 134, 42, 1, 138, 0, 1,
    138, 0, 1, 142, 0, 1, 142, 0, 2, 144, 43, 3, 144, 44, 5, 144, 45, 2, 128, 0, 0, 128, 0, 1, 130, 39, 1, 130, 0, 0,
    130, 0, 1, 148, 0, 1, 148, 0, 1, 148, 0, 2, 146, 40, 3, 146, 0, 3, 146, 0, 2, 146, 0, 1, 146, 0, 2, 38, 11, 2, 38,
    12, 3, 38, 13, 3, 38, 14, 3, 38, 15, 3, 38, 16, 3, 38, 17, 3, 38, 18, 3, 38, 19, 2, 38, 20, 3, 38, 21, 3, 38, 22, 3,
    38, 23, 3, 38, 24, 3, 38, 25, 3, 38, 26, 3, 38, 27, 3, 38, 28, 3, 38, 29, 2, 38, 30, 3, 38, 31, 3, 38, 32, 5, 38,
    33, 3, 38, 34, 2, 38, 35, 1, 38, 0, 1, 38, 0, 0, 108, 4, 1, 108, 5, 3, 108, 6, 1, 152, 0, 2, 152, 3, 1, 150, 7, 1,
    150, 8, 1, 150, 9, 1, 150, 10, 2, 154, 0, 2, 156, 50, 2, 156, 51, 3, 156, 52, 3, 156, 53, 3, 156, 54, 3, 156, 55, 3,
    156, 56, 3, 156, 57, 3, 156, 58, 2, 156, 59, 3, 156, 60, 3, 156, 61, 3, 156, 62, 3, 156, 63, 3, 156, 64, 3, 156, 65,
    3, 156, 66, 3, 156, 68, 3, 156, 67, 2, 156, 69, 3, 156, 70, 3, 156, 71, 5, 156, 72, 3, 156, 73, 0, 208, 74, 5, 156,
    75, 1, 156, 0, 1, 156, 0
};

static int goto_list[210] = {
    -1, 0, 209, 210, -1, 62, -1, 63, -1, 209, 68, 205, -1, 185, -1, 70, 70, 90, 119, 167, 122, 124, 125, 126, 168, 169,
    170, 171, -1, 95, 191, 193, -1, 186, 187, 188, -1, 177, 94, 96, 105, 200, 106, 198, 107, 197, 108, 196, 110, 195,
    127, 166, 128, 165, 129, 164, 130, 163, 131, 162, 132, 161, 133, 160, 134, 159, 135, 156, 136, 155, 137, 154, 138,
    153, 139, 152, 140, 151, 141, 150, 142, 149, 143, 148, 144, 147, 145, 146, 157, 158, 180, 181, 189, 190, -1, 121,
    -1, 174, 182, 184, -1, 175, 88, 89, 178, 179, -1, 79, 78, 80, 88, 80, 178, 80, -1, 97, -1, 74, -1, 87, -1, 73, -1,
    75, 84, 85, -1, 76, -1, 94, -1, 98, -1, 93, 170, 172, -1, 99, 118, 168, -1, 122, -1, 173, -1, 100, -1, 101, -1, 102,
    -1, 103, -1, 104, -1, 0, 3, 60, 4, 58, 5, 57, 6, 56, 7, 15, 16, 55, 17, 54, 18, 53, 19, 52, 20, 51, 21, 50, 22, 49,
    23, 48, 24, 45, 25, 44, 26, 43, 27, 42, 28, 41, 29, 40, 30, 39, 31, 38, 32, 37, 33, 36, 34, 35, 46, 47, -1, 2, -1,
    11
};

static int parse(int tt, int* sptr0, int** p_sptr, int rise_error) {
    enum { kShiftFlag = 1, kFlagCount = 1 };
    int action = rise_error;
    if (action >= 0) {
        const int* action_tbl = &action_list[action_idx[*(*p_sptr - 1)]];
        while (action_tbl[0] >= 0 && action_tbl[0] != tt) { action_tbl += 2; }
        action = action_tbl[1];
    }
    if (action >= 0) {
        if (!(action & kShiftFlag)) {
            const int* info = &reduce_info[action >> kFlagCount];
            const int* goto_tbl = &goto_list[info[1]];
            int state = *((*p_sptr -= info[0]) - 1);
            while (goto_tbl[0] >= 0 && goto_tbl[0] != state) { goto_tbl += 2; }
            *(*p_sptr)++ = goto_tbl[1];
            return predef_act_reduce + info[2];
        }
        *(*p_sptr)++ = action >> kFlagCount;
        return predef_act_shift;
    }
    /* Roll back to state, which can accept error */
    do {
        const int* action_tbl = &action_list[action_idx[*(*p_sptr - 1)]];
        while (action_tbl[0] >= 0 && action_tbl[0] != predef_tt_error) { action_tbl += 2; }
        if (action_tbl[1] >= 0 && (action_tbl[1] & kShiftFlag)) { /* Can recover */
            *(*p_sptr)++ = action_tbl[1] >> kFlagCount;           /* Shift error token */
            break;
        }
    } while (--*p_sptr != sptr0);
    return action;
}
