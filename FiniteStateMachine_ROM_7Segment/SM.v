
`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Module Name: SM for State Machine Chart, uses ROM to implement a SM chart
//////////////////////////////////////////////////////////////////////////////////
module SM (
input CLK,
input X3,
input X2,
input X1,
input PBC,
input CLK_Speed,
output Z3,
output Z2,
output Z1,
output q1,
output q0,
output RGB1_Red,
output reg[7:0] SSEG_CA,
output reg [7:0] SSEG_AN );
reg Slow_Clk, FF_Clk, Blink;
reg [4:0]ROM[0:31];
wire [4:0]index;
wire [3:0]SW;
reg [30:0] Divide;
wire q1P, q0P; //These are the outputs of the ROM connected to wires
initial //Fill the ROM with the required values for outs and NS
begin
Divide = 30'h00000000;
ROM[0] = 5'b01001;
ROM[1] = 5'b01001;
ROM[2] = 5'b10110;
ROM[3] = 5'b00101;
ROM[4] = 5'b01001;
ROM[5] = 5'b01001;
ROM[6] = 5'b10110;
ROM[7] = 5'b00110;
ROM[8] = 5'b00101;
ROM[9] = 5'b00101;
ROM[10] = 5'b00110;
ROM[11] = 5'b00110;
ROM[12] = 5'b00101;
ROM[13] = 5'b00101;
ROM[14] = 5'b00110;
ROM[15] = 5'b00110;
ROM[16] = 5'b00101;
ROM[17] = 5'b00100;
ROM[18] = 5'b00101;
ROM[19] = 5'b00100;
ROM[20] = 5'b00101;
ROM[21] = 5'b00100;
ROM[22] = 5'b00101;
ROM[23] = 5'b00100;
ROM[24] = 5'b00000;
ROM[25] = 5'b00000;
ROM[26] = 5'b00000;
ROM[27] = 5'b00000;
ROM[28] = 5'b00000;
ROM[29] = 5'b00000;
ROM[30] = 5'b00000;
ROM[31] = 5'b00000;
end
assign index = {q1,q0,X3,X2,X1};
assign q1P = ROM [index][1];
assign q0P = ROM [index][0];
assign SW = {2'b00,q1,q0}; //current state is q1q0
assign RGB1_Red = Blink;
assign Z1 = ROM [index][2];
assign Z2 = ROM [index][3];
assign Z3 = ROM [index][4];
always @(posedge CLK) begin
Divide <= Divide +1;
Slow_Clk <= Divide[25];
end
always @(posedge Slow_Clk) begin
Blink <= !Blink;
if(PBC) FF_Clk <= ! FF_Clk; //This creates a clock that advances only on the center button push
end
always @(SW)begin
SSEG_AN = 8'b11111110; //Turn on only the 1st digit
case (SW)
4'h0: SSEG_CA = ~(8'b00111111); //Note: to lite digit, cathode must = 0
4'h1: SSEG_CA = ~(8'b00000110);
4'h2: SSEG_CA = ~(8'b01011011);
4'h3: SSEG_CA = ~(8'b01001111);
4'h4: SSEG_CA = ~(8'b01100110);
4'h5: SSEG_CA = ~(8'b01101101);
4'h6: SSEG_CA = ~(8'b01111101);
4'h7: SSEG_CA = ~(8'b00000111);
4'h8: SSEG_CA = ~(8'b01111111);
4'h9: SSEG_CA = ~(8'b01100111);
4'hA: SSEG_CA = ~(8'b01110111);
4'hB: SSEG_CA = ~(8'b01111100);
4'hC: SSEG_CA = ~(8'b00111001);
4'hD: SSEG_CA = ~(8'b01011110);
4'hE: SSEG_CA = ~(8'b01111001);
4'hF: SSEG_CA = ~(8'b01110001);
default: SSEG_CA = ~(8'b01001001);
endcase
end
DFF In0 (q0P, FF_Clk, q0);
DFF In1 (q1P, FF_Clk, q1);
endmodule
module DFF (D, Clk, Q);
input D, Clk;
output reg Q;
reg Q ;
always @ (posedge Clk)begin
Q <= D;
end;
endmodule
 