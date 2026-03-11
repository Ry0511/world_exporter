//
// Date       : 11/03/2026
// Project    : world_exporter
// Author     : -Ry
//

#include "world_exporter/cpp/helpers.h"

namespace world_exporter::helpers {

FVector transform_point(const FMatrix& mat, const FVector& point) {
    const auto& M = mat.M;
    FPlane p{point.X, point.Y, point.Z, 1.0F};
    p.X = point.X * M[0][0] + point.Y * M[1][0] + point.Z * M[2][0] + 1.0F * M[3][0];
    p.Y = point.X * M[0][1] + point.Y * M[1][1] + point.Z * M[2][1] + 1.0F * M[3][1];
    p.Z = point.X * M[0][2] + point.Y * M[1][2] + point.Z * M[2][2] + 1.0F * M[3][2];
    p.W = point.X * M[0][3] + point.Y * M[1][3] + point.Z * M[2][3] + 1.0F * M[3][3];
    if (p.W != 0.0F) {
        return FVector{p.X / p.W, p.Y / p.W, p.Z / p.W};
    }
    return FVector{p.X, p.Y, p.Z};
}

}  // namespace world_exporter::helpers