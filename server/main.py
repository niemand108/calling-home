import datetime
import socketserver
import traceback
import subprocess
HOST = '192.168.0.178'
PORT = 58870
MIN_TIME_TO_RESET_CALLINGS = 11*60*1000
MIN_TIME_BW_CALLINGS = 60*1000


callings = []
updates = []

class DatagramInbound:
    @staticmethod
    def setup(host, port):
        print("UDP Inbound starts listening on {}:{}".format(host, port))

    @staticmethod
    def serve(host, port):
        try:
            datagram_inbound = socketserver.ThreadingUDPServer((host, port), DatagramHandler)
            DatagramInbound.setup(host, port)
            datagram_inbound.serve_forever()
            return datagram_inbound
        except OSError as exc:
            print("[Error]:", exc)
            raise exec

class DatagramHandler(socketserver.BaseRequestHandler):
    def handle(self):
        try:
            wire_data = self.request[0].strip().decode()
            
            if len(wire_data)>50:
                output("max len warning {}:{} (len = {})".format(*self.client_address,len(wire_data)))
                return None

            slices = wire_data.split(',')
            if len(slices)>3 or len(slices)<=1:
                output("max/min slices warning {}:{} (len = {})".format(*self.client_address,len(slices)))
                return None
            

            slices[0] = slices[0].strip()

            if not slices[0] in ["CALLING", "UPDATE"]:
                output("not command warning {}:{} (command = {})".format(*self.client_address,slices[0]))
                return None
            
            global callings, updates

            if slices[0] == 'CALLING':
                time_call = int(slices[1].strip())
                add_this_call_log = False
                if len(callings) == 0:
                    callings.append(time_call)
                    add_this_call_log = True
                else:
                    last_call = callings[-1]
                    if (last_call + MIN_TIME_BW_CALLINGS) <= time_call:
                        callings.append(time_call)
                        add_this_call_log = True
                if add_this_call_log:
                    subprocess.Popen(["mpg123","ring.mp3"])
                    output("CALLING " + str(time_call))
                    output("conn -> {}:{}".format(*self.client_address))
                    output("recv -> ({}) from {}:{}".format(wire_data, *self.client_address))
            elif slices[0] == 'UPDATE':
                time_update = int(slices[1].strip())
                updates.append(time_update)
                
                if time_update < MIN_TIME_TO_RESET_CALLINGS:
                    callings.clear()
                    output("UPDATE clear callings (update_now = {}) from {}:{}".format(time_update,*self.client_address))
                    #callings.append(time_update)
                output("UPDATE ({}) -> ({}) from {}:{}".format(time_update,wire_data, *self.client_address))
                if len(slices)>=3:
                    warning_extra = slices[2].strip()
                    output(" **** UPDATE **** " + str(warning_extra))
            return None
        except Exception as e:
            lines = traceback.format_exception(type(e), e, e.__traceback__)
            output("recv exception-> ({}) from {}:{}".format(wire_data, *self.client_address))
            output(''.join(lines))
    
def output(message, filename=None):
    now = datetime.datetime.now()
    print("[{}] {}".format(now,message))

if __name__ == "__main__":
    try:
        DatagramInbound.serve(HOST, PORT)
    except:
        import sys,os
        sys.exit(os.EX_SOFTWARE)