`timescale 1ns / 1ps

module Decoder3to8(in, out);
    input [2:0] in;
    output reg [7:0] out;
    
    always @(*) begin
        case(in)
            3'b000: out <= 8'b00000001;
            3'b001: out <= 8'b00000010;
            3'b010: out <= 8'b00000100;
            3'b011: out <= 8'b00001000;
            3'b100: out <= 8'b00010000;
            3'b101: out <= 8'b00100000;
            3'b110: out <= 8'b01000000;
            3'b111: out <= 8'b10000000;
            default: out <= 8'b00000000;
          endcase
  end
endmodule


module decoder_testbench();
    reg [2:0] in;
    wire [7:0] out;
    
    Decoder uut(in, out);
    
    initial begin
        $monitor("in=%b, out=%b", in, out);
        in = 3'b000;
        #100;
        in = 3'b001;
        #100;
        in = 3'b010;
        #100;
        in = 3'b011;
        #100;
        in = 3'b100;
        #100;
        in = 3'b101;
        #100;
        in = 3'b110;
        #100;
        in = 3'b111;
      end
 

endmodule
