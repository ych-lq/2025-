
set -e

CIRCOM_FILE="poseidon2_hash.circom"
INPUT_FILE="input.json"
OUTPUT_DIR="poseidon2_hash_js"

if [[ ! -e $CIRCOM_FILE ]]; then
    echo "错误$CIRCOM_FILE"
    exit 1
fi

if [[ ! -e $INPUT_FILE ]]; then
    echo "错误 $INPUT_FILE"
    exit 1
fi

circom "$CIRCOM_FILE" --r1cs --wasm --sym --c

if [[ ! -d $OUTPUT_DIR ]]; then
    echo "错误: 编译失败 $OUTPUT_DIR"
    exit 1
fi

cd "$OUTPUT_DIR" || exit
node generate_witness.js poseidon2_hash.wasm "../$INPUT_FILE" "../witness.wtns"
cd ..

snarkjs r1cs info poseidon2_hash.r1cs

snarkjs r1cs print poseidon2_hash.r1cs poseidon2_hash.sym

