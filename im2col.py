from builtins import range
import numpy as np


def get_im2col_indices(x_shape, field_height, field_width, stride=1):
    # First figure out what the size of the output should be
    H, W = x_shape
    assert (H - field_height) % stride == 0
    assert (W - field_height) % stride == 0
    out_height = (H - field_height) // stride + 1
    out_width = (W - field_width) // stride + 1

    i0 = np.repeat(np.arange(field_width), field_height)
    i1 = stride * np.repeat(np.arange(out_width), out_height)
    j0 = np.tile(np.arange(field_height), field_width)
    j1 = stride * np.tile(np.arange(out_height), out_width)
    i = i0.reshape(1, -1) + i1.reshape(-1, 1)
    j = j0.reshape(1, -1) + j1.reshape(-1, 1)

    return (i, j)


def im2col_indices(x, field_height, field_width, stride=1):
    """ An implementation of im2col based on some fancy indexing """
    i, j = get_im2col_indices(x.shape, field_height, field_width, stride)

    cols = x[i, j]
    return cols


def col2im_indices(cols, x_shape, field_height=3, field_width=3, padding=1, stride=1):
    """ An implementation of col2im based on fancy indexing and np.add.at """
    N, C, H, W = x_shape
    H_padded, W_padded = H + 2 * padding, W + 2 * padding
    x_padded = np.zeros((N, C, H_padded, W_padded), dtype=cols.dtype)
    k, i, j = get_im2col_indices(x_shape, field_height, field_width, padding, stride)
    cols_reshaped = cols.reshape(C * field_height * field_width, -1, N)
    cols_reshaped = cols_reshaped.transpose(2, 0, 1)
    np.add.at(x_padded, (slice(None), k, i, j), cols_reshaped)
    if padding == 0:
        return x_padded
    return x_padded[:, :, padding:-padding, padding:-padding]


# *****START OF YOUR CODE (DO NOT DELETE/MODIFY THIS LINE)*****

if (__name__ == '__main__'):
    # Test im2col and col2im
    x_shape = (4, 4)
    # x = np.random.randn(*x_shape)
    x = np.arange(16).reshape(4, 4)
    x_col = im2col_indices(x, 3, 3, stride=1)
    print(x_col)
    x_col_reshaped = x_col.reshape(3 * 3 * 3, -1)
    x_reconstructed = col2im_indices(x_col, x_shape, 3, 3, padding=1, stride=1)
    print('Reconstruction error:', np.linalg.norm(x - x_reconstructed))

    # Test get_im2col_indices
    k, i, j = get_im2col_indices((2, 3, 4, 4), 3, 3, padding=1, stride=1)
    print('k:', k)
    print('i:', i)
    print('j:', j)

    # Test im2col_indices
    x_shape = (2, 3, 4, 4)
    x = np.random.randn(*x_shape)
    x_col = im2col_indices(x, 3, 3, padding=1, stride=1)
    print('x_col:', x_col)

    # Test col2im_indices
    x_shape = (2, 3, 4, 4)
    x = np.random.randn(*x_shape)
    x_col = im2col_indices(x, 3, 3, padding=1, stride=1)
    x_reconstructed = col2im_indices(x_col, x_shape, 3, 3, padding=1, stride=1)
    print('Reconstruction error:', np.linalg.norm(x - x_reconstructed))

    # Test col2im_indices
    x_shape = (2, 3, 4, 4)
    x = np.random.randn(*x_shape)
    x_col = im2col_indices(x, 3, 3, padding=1, stride=1)
    x_reconstructed = col2im_indices(x_col, x_shape, 3, 3, padding=1, stride=1)
    print('Reconstruction error:', np.linalg.norm(x - x_reconstructed))

    # Test col2im_indices
    x_shape = (2, 3, 4, 4)

# *****END OF YOUR CODE (DO NOT DELETE/MODIFY THIS LINE)*****
