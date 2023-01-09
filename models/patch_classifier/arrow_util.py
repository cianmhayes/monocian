import pyarrow as pa
import struct
import torch


def float_tensor_to_arrow_array(input: torch.Tensor) -> pa.Array:
    assert len(input.shape) == 2
    features = input.shape[1]
    pack_format = "<" + ("f" * features)
    return pa.array([struct.pack(pack_format, *t.tolist()) for t in input])


def arrow_bytes_array_to_float_tensor(
    input: pa.Array, expected_features: int
) -> torch.Tensor:
    pack_format = "<" + ("f" * expected_features)
    tensors = []
    for row in input:
        tensor = torch.Tensor(struct.unpack(pack_format, row.as_py()))
        tensors.append(tensor)
    return torch.stack(tensors)


def arrow_int_array_to_long_tensor(input: pa.Array) -> torch.LongTensor:
    return torch.LongTensor(input.to_pylist())
