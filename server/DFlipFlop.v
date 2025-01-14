`timescale 1ns / 1ps


module DFlipFlop(CLK_signal, D, RESET, Q, Q_NOT);
    input CLK_signal, D, RESET;
    output reg Q, Q_NOT;

    always@(posedge CLK_signal)begin
        if(RESET)begin
            Q <= 1'b0;
            Q_NOT <= 1'b1;
        end
        else begin
           Q <= D;
           Q_NOT <= ~D;

        end
    end
endmodule

module DFlipFlop_tb();
    reg CLK_signal, D, RESET;
    wire Q, Q_NOT;

    DDFF uut(CLK_signal, D, RESET, Q, Q_NOT);
    initial begin
         CLK_signal = 1'b0;
         forever begin
             #10;
             CLK_signal = !CLK_signal;
         end
     end
     initial begin
         $monitor("CLK_signal=%b, D=%b, RESET=%b, Q=%b, Q_NOT=%b", CLK_signal, D, RESET, Q, Q_NOT);
         D = 1'b0;
         RESET = 1'b0;
         #100;
         D = 1'b1; 
         RESET = 1'b0;
         #100;
         D = 1'bX;
         RESET = 1'b1;
         #100;
       end
endmodule
         