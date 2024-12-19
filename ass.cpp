#include <iostream>
#include <eigen/Eigen/Dense>
#include <vector>
#include <cassert>

using namespace Eigen;

// Helper function to mimic numpy's repeat
VectorXi repeat(const VectorXi& vec, int n) {
    VectorXi result(vec.size() * n);
    for (int i = 0; i < vec.size(); ++i) {
        result.segment(i*n, n).setConstant(vec(i));
    }
    return result;
}

// Helper function to mimic numpy's arange
VectorXi arange(int n) {
    return VectorXi::LinSpaced(n, 0, n-1);
}

// Helper function to mimic numpy's tile
MatrixXi tile(const VectorXi& vec, int n) {
    return vec.replicate(1, n);
}

std::pair<MatrixXi, MatrixXi> get_im2col_indices(const std::pair<int, int>& x_shape, 
                                                 int field_height, int field_width, int stride = 1) {
    int H = x_shape.first;
    int W = x_shape.second;
    
    assert((H - field_height) % stride == 0);
    assert((W - field_width) % stride == 0);
    
    int out_height = (H - field_height) / stride + 1;
    int out_width = (W - field_width) / stride + 1;

    VectorXi i0 = repeat(arange(field_width), field_height);
    VectorXi i1 = stride * repeat(arange(out_width), out_height);
    VectorXi j0 = tile(arange(field_height), field_width);
    VectorXi j1 = stride * tile(arange(out_height), out_width);

    MatrixXi i = i0.replicate(1, i1.size()) + i1.replicate(i0.size(), 1);
    MatrixXi j = j0.replicate(1, j1.size()) + j1.replicate(j0.size(), 1);

    return std::make_pair(i, j);
}

MatrixXf im2col_indices(const MatrixXf& x, int field_height, int field_width, int stride = 1) {
    auto indices = get_im2col_indices(std::make_pair(x.rows(), x.cols()), 
                                      field_height, field_width, stride);
    
    MatrixXi i = indices.first;
    MatrixXi j = indices.second;

    MatrixXf cols(i.rows() * i.cols(), 1);
    for (int idx = 0; idx < i.size(); ++idx) {
        cols(idx) = x(i(idx), j(idx));
    }

    return cols;
}

int main() {
    MatrixXf x(4, 4);
    x << 1, 2, 3, 4,
         5, 6, 7, 8,
         9, 10, 11, 12,
         13, 14, 15, 16;

    MatrixXf cols = im2col_indices(x, 2, 2, 1);
    std::cout << cols << std::endl;
    return 0;
}