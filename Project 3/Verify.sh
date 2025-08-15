const snarkjs = require("snarkjs");
const fs = require("fs");

async function run() {
    const data = JSON.parse(fs.readFileSync("input.json"));

    const { proof, publicSignals } = await snarkjs.groth16.prove("poseidon2_hash.zkey", "witness.wtns");

    const verificationKey = JSON.parse(fs.readFileSync("verify.json"));
    const isValid = await snarkjs.groth16.verify(verificationKey, publicSignals, proof);

    if (isValid) {
        console.log("Verification succeeded!");
    } else {
        console.log("Verification failed!");
    }
}
run().then(() => {
    process.exit(0);
}).catch((error) => {
    console.error(error);
    process.exit(1);
});
