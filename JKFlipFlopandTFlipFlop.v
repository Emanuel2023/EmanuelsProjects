`timescale 1ns / 1ps


module JK_FF(CLK, J, K, Q, Q_NOT);
    input CLK, J, K;
    output reg Q, Q_NOT;
    
    always @(posedge CLK) begin
        case ({J,K})
            2'b00: {Q, Q_NOT} <= {Q, Q_NOT};//HOLD
            2'b01: {Q, Q_NOT} <= {1'b0, 1'b1};//RESET
            2'b10: {Q, Q_NOT} <= {1'b1, 1'b0};//SET
            2'b11: {Q, Q_NOT} <= {Q_NOT, Q};
        endcase
    end
endmodule

//module T_FF(CLK, T, RST, Q, Q_NOT);
//    input CLK, T, RST;
//    output reg Q, Q_NOT;
    
//    always @(posedge CLK) begin
//        if(RST) begin
//            Q <= 1'b0;
//            Q_NOT <= 1'b1;
//        end
//        else begin 
//            if(T) begin
//                Q <= ~Q;
//                Q_NOT <= Q;
//            end
//        end
//    end
//endmodule


`timescale 1ns / 1ps


module JK_FF_tb();
    reg CLK, J, K;
    wire Q, Q_NOT;
    
    JK_FF uut(CLK, J, K, Q, Q_NOT);
    
    initial begin
        CLK = 1'b0;
        forever begin
            #10;
            CLK = !CLK;
        end
      end
    initial begin 
        $monitor("CLK=%b, J=%b, K=%b, Q=%b, Q_NOT=%b", CLK, J, K, Q, Q_NOT);
        #100;
        J = 1'b0;
        K = 1'b0;
    
        #100;
        J = 1'b1;
        K = 1'b0;
    
        #100;
        J = 1'b0;
        K = 1'b1;
    
        #100;
        J = 1'b1;
        K = 1'b1;
        
        #100;
        
    

     end
endmodule







//`timescale 1ns / 1ps


//module T_FF_tb();
//    reg CLK, T, RST;
//    wire Q, Q_NOT;
    
//    T_FF uut(CLK, T, RST, Q, Q_NOT);
    
//    initial begin
//        CLK = 1'b0;
//        forever begin
//            #10;
//            CLK = !CLK;
//        end
//      end
//    initial begin 
//        $monitor("T=%b, RST=%b, Q=%b, Q_NOT=%b", T, RST, Q, Q_NOT);
//        T = 1'b0;
//        RST = 1'b0;
        
//        // Apply test vectors with appropriate delays
//        #100;
//        T = 1'b1;
//        #100;
//        RST = 1'b1;
//        #100;
//        RST = 1'b0;
//        #100;
//        T = 1'b0;

//     end
//endmodule
