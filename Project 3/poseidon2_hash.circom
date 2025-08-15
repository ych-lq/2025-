pragma circom 2.1.6;

include "/home/seed/Desktop/circom/parser/circomlib-master/circuits/poseidon.circom";

template Poseidon2() {
    signal input inputs[2];
    signal output output;

    component hash = Poseidon(2);

    for (var index = 0; index < 2; index++) {
        hash.inputs[index] <== inputs[index];
    }

    output <== hash.out;
}

component mainCircuit = Poseidon2();
