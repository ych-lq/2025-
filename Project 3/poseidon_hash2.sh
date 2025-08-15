circom poseidon_hash2.circom --r1cs --wasm --sym -o build
snarkjs r1cs info build/poseidon2_t3.r1cs
snarkjs powersoftau new bn128 14 pot14_0000.ptau -v
snarkjs powersoftau contribute pot14_0000.ptau pot14_0001.ptau --name="first" -v
snarkjs powersoftau prepare phase2 pot14_0001.ptau pot14_final.ptau
snarkjs groth16 setup build/poseidon_hash2.rlcs pot14_final.ptau build/poseidon_hash2.zkey
snarkjs zkey export verificationkey build/poseidon_hash2.zkey build/verification_key.json
cat > input.json << 'EOF'
{ "in0": "1", "in1": "2", "in2": "3" }
EOF
snarkjs wtns calculate build/poseidon_hash2_js/poseidon_hash2.wasm input.json build/witness.wtns
ls -l build/witness.wtns
snarkjs groth16 prove build/poseidon_hash2.zkey build/witness.wtns proof.json public.json
snarkjs groth16 verify build/verification_key.json public.json proof.json
