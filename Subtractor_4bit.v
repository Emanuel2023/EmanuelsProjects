`timescale 1ns / 1ps
module Subtractor_4(A, B, Bin, diff, Bout);
    input [3:0] A, B; // 4-bit inputs A and B
    input Bin; // Borrow input
    output [3:0] diff; // 4-bit difference output
    output Bout; // Borrow output
    wire [2:0] carryout; // Intermediate borrow signals for chaining
    full_subtractor s0(A[0], B[0], Bin, diff[0], carryout[0]);  // LSB subtractor
    full_subtractor s1(A[1], B[1], carryout[0], diff[1], carryout[1]); // Next bit subtractor
    full_subtractor s2(A[2], B[2], carryout[1], diff[2], carryout[2]); // Next bit subtractor
    full_subtractor s3(A[3], B[3], carryout[2], diff[3], Bout); // MSB subtractor
endmodule

module full_subtractor(A, B, Bin, diff, Bout);
    input A, B, Bin; // Inputs for the full subtractor
    output diff, Bout; // Outputs for the difference and borrow
    assign diff = A ^ B ^ Bin;  // Calculate the difference using XOR operation
    assign Bout = (B && Bin) || (!A && ((B && !Bin) || (!B && Bin))); // Calculate the borrow output
endmodule


module Subtractor_4_tb();
    reg [3:0] A, B;
    reg Bin;
    wire [3:0] diff;
    wire Bout;
    Subtractor_4 uut(A, B, Bin, diff, Bout);
initial begin
    $monitor("A=%b, B=%b, Bin=%b, diff=%b, Bout=%b", A, B, Bin, diff, Bout);
    A = 4'b0000;
    B = 4'b0000;
    Bin = 1'b0;
    #100;
    Bin = 1'b1;
    A = 4'b0000;
    B = 4'b0000;
    #100
    Bin = 1'b0;
    A = 4'b0000;
    B = 4'b0001;
    #100
    Bin = 1'b1;
    A = 8'b0000;
    B = 8'b0001;
    #100
    Bin = 1'b0;
    A = 8'b0001;
    B = 8'b0000;
    #100;

    Bin = 1'b1;
    A = 4'b0001;
    B = 4'b0000;
    #100
    Bin = 1'b0;
    A = 4'b0001;
    B = 4'b0001;
    #100
    Bin = 1'b1;
    A = 8'b0001;
    B = 8'b0001;
   end
endmodule