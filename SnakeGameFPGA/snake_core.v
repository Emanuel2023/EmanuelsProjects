
`timescale 1ns / 1ps
module snake_core (Left, Right, Up, Down, Ack, Reset, Clk, Qi, Qm, Qc, Qh, Qe,
                              Qw, Ql, Qu, Food, Length, Locations_Flat);

input Left, Right, Up, Down;
input Ack, Reset, Clk;

output Qi, Qm, Qc, Qh, Qe, Qw, Ql, Qu;
output [7:0] Food;
output [3:0] Length;
output [127:0] Locations_Flat; // 16 locations

reg [7:0] rand_loc;

reg [7:0] Food;
reg [3:0] Length;
reg [7:0] locations [15:0];

wire [127:0] Locations_Flat; // see top for explanation
assign Locations_Flat = {locations[0], locations[1], locations[2], locations[3],
                                    locations[4], locations[5], locations[6], locations[7],
                                    locations[8], locations[9], locations[10], locations[11],
                                    locations[12], locations[13], locations[14], locations[15]};

reg [7:0] state;
reg [1:0] next_dir;

integer i, j;

// one-hot state machine
localparam
      INIT  = 8'b00000001,
      MOVE  = 8'b00000010,
      CHECK = 8'b00000100,
      HOLD  = 8'b00001000,
      EAT   = 8'b00010000,
      WIN   = 8'b00100000,
      LOSE  = 8'b01000000,
      UNKN  = 8'b10000000;

assign {Qu, Ql, Qw, Qe, Qh, Qc, Qm, Qi} = state;

// next_dir assignments
localparam
      LEFT = 2'b00,
      RIGHT = 2'b01,
      UP = 2'b10,
      DOWN = 2'b11;

// record last direction pushed
always @(posedge Clk)
begin
      if (Left) begin
            next_dir <= LEFT;
      end
      else if (Right) begin
            next_dir <= RIGHT;
      end
      else if (Up) begin
            next_dir <= UP;
      end
      else if (Down) begin
            next_dir <= DOWN;
      end
end

// Pseudorandom food location generator
always @(posedge Clk)
begin
      // Do not need known initial state
      rand_loc <= rand_loc + 1;
end

always @(posedge Clk, posedge Reset) // asynchronous high-active reset
begin
      if (Reset) begin
            state <= INIT;
            // Send locations into known initial state
            locations[0] <= 8'b00000000;
            locations[1] <= 8'b00000000;
            locations[2] <= 8'b00000000;
            locations[3] <= 8'b00000000;
            locations[4] <= 8'b00000000;
            locations[5] <= 8'b00000000;
            locations[6] <= 8'b00000000;
            locations[7] <= 8'b00000000;
            locations[8] <= 8'b00000000;
            locations[9] <= 8'b00000000;
            locations[10] <= 8'b00000000;
            locations[11] <= 8'b00000000;
            locations[12] <= 8'b00000000;
            locations[13] <= 8'b00000000;
            locations[14] <= 8'b00000000;
            locations[15] <= 8'b00000000;
            locations[16] <=  8'b00000000;
            locations[17] <=  8'b00000000;
            locations[18] <=  8'b00000000;
            locations[19] <=  8'b00000000;
            Length <= 0;
            Food <= 0;
      end

      else begin
            case (state)
                  INIT: // when ack is received, start moving
                  begin
                        locations[0] <= 125;
                        locations[1] <= 124;
                        // So that snake resets on ACK
                        locations[2] <= 8'b00000000;
                        locations[3] <= 8'b00000000;
                        locations[4] <= 8'b00000000;
                        locations[5] <= 8'b00000000;
                        locations[6] <= 8'b00000000;
                        locations[7] <= 8'b00000000;
                        locations[8] <= 8'b00000000;
                        locations[9] <= 8'b00000000;
                        locations[10] <= 8'b00000000;
                        locations[11] <= 8'b00000000;
                        locations[12] <= 8'b00000000;
                        locations[13] <= 8'b00000000;
                        locations[14] <= 8'b00000000;
                        locations[15] <= 8'b00000000;
                        locations[16] <=  8'b00000000;
                        locations[17] <=  8'b00000000;
                        locations[18] <=  8'b00000000;
                        locations[19] <=  8'b00000000;
                        
                        
                        
                        Length <= 1;
                        if (Ack) begin
                              state <= EAT;
                        end
                  end

                  MOVE: // update the byte array of positions
                  begin
                        for (i = 14; i >= 0; i = i - 1) begin
                              if ((i <= Length - 1) && (i >= 0))
                              begin
                                 locations[i + 1] <= locations[i];
                              end
                        end

                        if (next_dir == LEFT) begin
                              locations[0] <= locations[0] - 1;
                        end

                        else if (next_dir == RIGHT) begin
                              locations[0] <= locations[0] + 1;
                        end

                        else if (next_dir == UP) begin
                              locations[0] <= locations[0] - 16;
                        end

                        else if (next_dir == DOWN) begin
                              locations[0] <= locations[0] + 16;
                        end

                        state <= CHECK;
                  end

                  CHECK: // check if we grow or if we lost
                  begin
                        if (locations[0] == Food) begin
                              state <= EAT;
                        end

                        
                        // checks if we lost
                        else begin
                              state <= HOLD;
                                  for (i = 1; i < Length; i = i + 1) begin
                  if (locations[0] == locations[i]) begin
                        state <= LOSE;
                  end
            end
      end
end
           

                  // grows the snake and moves the food
                  EAT:
                  begin
                        state <= MOVE;
                        Length <= Length + 1;
                        Food <= rand_loc;
                        if (Length == 19) begin
                              state <= WIN;
                        end
                  end

                  HOLD: // ensure constant gamespeed
                  begin
                        state <= MOVE;
                  end

                  WIN:
                  begin
                        if (Ack) begin
                              state <= INIT;
                        end
                  end

                  LOSE:
                  begin
                        if (Ack) begin
                              state <= INIT;
                        end
                  end

                  default:
                  begin
                        state <= UNKN;
                  end
            endcase // state
      end
end

endmodule