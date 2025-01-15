#Emanuel Contreras lct377

import subprocess
RETURN_CODE = 0 #constant declated for the return code

class CommandRunner:
    def run_command(self, command):
        try:
            process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True) #Will create the subprocess and will start it.
            output, error = process.communicate() #This will read the output and error streams
            
            if process.returncode != RETURN_CODE: #Checks if the return code is not the expected one. Will indicate an error
                print(f"Error: {error}")
                return None
            else:
                return output.decode() #will decode and return the output.
        except Exception as e:
            print(f"Error: {str(e)}") #This will take care of any expections that may occur.
            return None

    def parse_ls_output(self, output): #This will parse the output of 'ls' command and extract the file names
        lines = output.split('\n')
        file_names = []
        for line in lines[1:]:  # Skip the header line
            if line: #This will split the line and extract the file name.
                parts = line.split()
                file_names.append(parts[-1])
        return file_names
        

    def parse_ps_output(self, output): #This will parse the output of 'ps' command and extract command names
        lines = output.split('\n')
        command_names = []
        for line in lines[1:]:  # Skip the header line
            if line: #This will split the line and extract the command name.
                parts = line.split()
                command_names.append(parts[-1])
        return command_names
    
