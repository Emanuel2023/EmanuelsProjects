`timescale 1ns / 1ps

module TwoBitAdderWithROM(
    input [1:0] A,
    input [1:0] B,
    output [2:0] Sum
);
// ROM implementation here
    reg [2:0] rom_data [0:3];

    initial begin
        // Load the ROM with precomputed sum values
        rom_data[0] = 3'b00; // 00 + 00 = 00
        rom_data[1] = 3'b01; // 00 + 01 = 01
        rom_data[2] = 3'b01; // 01 + 00 = 01
        rom_data[3] = 3'b10; // 01 + 01 = 10
    end

    assign Sum = rom_data[(A << 2) | B];

endmodule

module nf(CLK100MHZ, SEG, AN, SW);
    input CLK100MHZ;
    input [3:0] SW;
    output [7:0] AN;
    output [6:0] SEG;

    wire Clk_Slow, Clk_Multi;

    parameter width1 = 100000000;
    parameter width2 = 10000;   

    Clk_Divide # (width1, width2) In1 (CLK100MHZ, Clk_Slow, Clk_Multi); 

    reg [6:0] SEG2;
    reg Counter;

    always @(posedge Clk_Multi) begin
        Counter <= !Counter;
    end

    always @(posedge Clk_Multi) begin
        if (Counter == 1'b0) SEG2 <= 7'b1000000;
        if (Counter == 1'b1) SEG2 <= 7'b1111001; 
    end

    // Instantiate the 2-Bit Adder with ROM
    TwoBitAdderWithROM adder_inst (
        .A(SW[1:0]),   // Connect SW[1:0] to the A input
        .B(SW[3:2]),   // Connect SW[3:2] to the B input
        .Sum(SEG2[2:0]) // Connect the adder's output to the lowest 3 bits of SEG2
    );

    assign AN[0] = Counter;
    assign AN[1] = ~Counter;
    assign AN[7:2] = 6'b111111;
    assign SEG = SEG2;

endmodule

module Clk_Divide (Clk, Clk_Slow, Clk_multi);
    input Clk;
    output Clk_Slow, Clk_multi;
    reg Clk_Slow;
    reg Clk_multi;
    parameter size1 = 100000000;
    parameter size2 = 10000;

    reg [31:0] counter_out1, counter_out2;

    initial begin
        counter_out1 <= 32'h00000000;
        counter_out2 <= 32'h00000000;
        Clk_Slow <= 1'b0;
        Clk_multi <= 1'b0;
    end

    always @(posedge Clk) begin
        counter_out1 <= counter_out1 + 32'h00000001;
        counter_out2 <= counter_out2 + 32'h00000001;
        
        if (counter_out1 > size1) begin
            counter_out1 <= 32'h00000000;
            Clk_Slow <= !Clk_Slow;
        end
        if (counter_out2 > size2) begin
            counter_out2 <= 32'h00000000;
            Clk_multi <= !Clk_multi;
        end
    end

endmodule