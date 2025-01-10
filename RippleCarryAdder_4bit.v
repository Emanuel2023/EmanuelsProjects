
`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 09/04/2024 11:53:32 PM
// Design Name: 
// Module Name: RippleCarryAdder
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module RippleCarryAdder (A, B, Cin, Sum, Cout);
    input [3:0] A, B;         // 4-bit input operand B
    input Cin;         // Carry-in bit
    output [3:0] Sum;    // 4-bit sum output
    output Cout;            // Carry-out bit


    // Internal carry signals
    wire carryout1, carryout2, carryout3;

    // Instantiate the full adders
    full_adder fa0 (.A(A[0]), .B(B[0]), .Cin(Cin), .Sum(Sum[0]), .Cout(carryout1));
    full_adder fa1 (.A(A[1]), .B(B[1]), .Cin(carryout1), .Sum(Sum[1]), .Cout(carryout2));
    full_adder fa2 (.A(A[2]), .B(B[2]), .Cin(carryout2), .Sum(Sum[2]), .Cout(carryout3));
    full_adder fa3 (.A(A[3]), .B(B[3]), .Cin(carryout3), .Sum(Sum[3]), .Cout(Cout));


endmodule


module full_adder (
    input A,       // Input bit A
    input B,       // Input bit B
    input Cin,     // Carry-in bit
    output Sum,    // Sum output
    output Cout    // Carry-out output
);

    assign Sum = A ^ B ^ Cin;                   // Sum calculation
    assign Cout = (A & B) | (Cin & (A ^ B));   // Carry-out calculation

endmodule



module RippleCarryAdder_tb;

    // Declare the registers and wires for the testbench
    reg [3:0] A, B;       // 4-bit input operands
    reg Cin;              // Carry-in bit
    wire [3:0] Sum;      // 4-bit sum output
    wire Cout;             // Carry-out bit

    // Instantiate the rca module
    RippleCarryAdder uut(A, B, Cin, Sum, Cout);

    // Initial block for stimulus
    initial begin
        // Initialize inputs
        Cin = 1;
        A = 4'b0110;
        B = 4'b1100;

        // Monitor changes and display results
        $monitor("A=%b, B=%b, Cin=%b, Sum=%b, Cout=%b", A, B, Cin, Sum, Cout);

        // Apply test vectors
        #100;
        A = 4'b1110;
        B = 4'b1000;

        #100;
        A = 4'b0111;
        B = 4'b1110;

        #100;
        A = 4'b0010;
        B = 4'b1001;

        #100;
        // End simulation
        
    end

endmodule
