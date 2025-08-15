
set -e

CIRCOM_FILE="poseidon_hash2.circom"
INPUT_FILE="input.json"
OUTPUT_DIR="poseidon_hash2_js"

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
node generate_witness.js poseidon_hash2.wasm "../$INPUT_FILE" "../witness.wtns"
cd ..

snarkjs r1cs info poseidon_hash2.r1cs

snarkjs r1cs print poseidon_hash2.r1cs poseidon_hash2.sym

