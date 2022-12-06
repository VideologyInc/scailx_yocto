#!/usr/bin/env python

# https://www.youtube.com/watch?v=dgvLegLW6ek

# webserver.py
# https://www.youtube.com/watch?v=Lbfe3-v7yE0
# server.py client.py

from http.server import HTTPServer, BaseHTTPRequestHandler
import cgi
import serial

tasklist = ['Task 1', 'Task 2', 'Task 3']

class requestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path.startswith('/'):
            self.send_response(200)
            self.send_header('content-type', 'text/html')
            self.end_headers()

            output =''
            output += '<html><body>'
            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_Zoom/Tele">'
            output += '<input type = "submit" value="Tele">'
            output += '</form>'
            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_Zoom/Wide">'
            output += '<input type = "submit" value="Wide">'
            output += '</form>'
            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_Zoom/Stop">'
            output += '<input type = "submit" value="Stop">'
            output += '</form>'
            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_MENUKey/MENU">'
            output += '<input type = "submit" value="MENU">'
            output += '</form>'
            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_MENUKey/ESC">'
            output += '<input type = "submit" value="ESC">'
            output += '</form>'
            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_MENUKey/UP">'
            output += '<input type = "submit" value="UP">'
            output += '</form>'
            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_MENUKey/DOWN">'
            output += '<input type = "submit" value="DOWN">'
            output += '</form>'
            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_MENUKey/LEFT">'
            output += '<input type = "submit" value="LEFT">'
            output += '</form>'
            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_MENUKey/RIGHT">'
            output += '<input type = "submit" value="RIGHT">'
            output += '</form>'
            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_POWER">'
            output += '<input type = "submit" value="RESET">'
            output += '</form>'
            output += '</form>'
            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_LR_REVERSE/ON">'
            output += '<input type = "submit" value="MIRROR ON">'
            output += '</form>'
            output += '</form>'
            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_LR_REVERSE/OFF">'
            output += '<input type = "submit" value="MIRROR OFF">'
            output += '</form>'
            output += '</form>'
            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_PICTURE_FLIP/ON">'
            output += '<input type = "submit" value="FLIP ON">'
            output += '</form>'
            output += '</form>'
            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_PICTURE_FLIP/OFF">'
            output += '<input type = "submit" value="FLIP OFF">'
            output += '</form>'
            output += '</form>'
            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_AF_MODE/NORMAL_AF">'
            output += '<input type = "submit" value="NORMAL AF">'
            output += '</form>'
            output += '</form>'
            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_AF_MODE/INTERVAL_AF">'
            output += '<input type = "submit" value="INTERVAL AF">'
            output += '</form>'
            output += '</form>'
            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_AF_MODE/ZOOM_TRIG_AF">'
            output += '<input type = "submit" value="ZOOM AF">'
            output += '</form>'
            output += '</body></html>'
#            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_WDR/ON">'
#            output += '<input type = "submit" value="WDR ON">'
#            output += '</form>'
#            output += '</body></html>'
#            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_WDR/OFF">'
#            output += '<input type = "submit" value="WDR OFF">'
#            output += '</form>'
#            output += '</body></html>'
            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_ICR/NIGHT">'
            output += '<input type = "submit" value="ICR NIGHT">'
            output += '</form>'
            output += '</body></html>'
            output += '<form method="POST" enctype="multipart/form-data" action="/imx8/CAM_ICR/DAY">'
            output += '<input type = "submit" value="ICR DAY">'
            output += '</form>'
            output += '</body></html>'
            self.wfile.write(output.encode())


        if self.path.endswith('/tasklist'):
            self.send_response(200)
            self.send_header('content-type', 'text/html')
            self.end_headers()

            output =''
            output += '<html><body>'
            output += '<h1>Task List</h1>'
            output += '<h3><a href="/tasklist/new">Add New Task</a></h3>'
            for task in tasklist:
                output += task
                output += '<a href="/tasklist/%s/remove">X</a>' % task
                output += '</br>'
            output += '</body></html>'
            self.wfile.write(output.encode())

        if self.path.endswith('/remove'):
            listIDPath = self.path.split('/')[2]
            print(listIDPath)
            self.send_response(200)
            self.send_header('content-type', 'text/html')
            self.end_headers()

            output =''
            output += '<html><body>'
            output += '<h1>Remove task: %s</h1>' % listIDPath.replace('%20', ' ')
            output += '<form method = "POST" enctype = "multipart/form-data" action = "/tasklist/%s/remove">' % listIDPath
            output += '<input type = "submit" value="Remove"></form>'
            output += '<a href="/tasklist">Cancel</a>'
            output += '</body></html>'
            self.wfile.write(output.encode())


        if self.path.endswith('/new'):
            self.send_response(200)
            self.send_header('content-type', 'text/html')
            self.end_headers()

            output = ''
            output += '<html><body>'
            output += '<h1>Add New Task</h1>'

            output += '<form method="POST" enctype="multipart/form-data" action="/tasklist/new">'
            output += '<input name="task" type="text" placeholder="Add new task">'
            output += '<input type="submit" value="Add">'
            output += '</form>'
            output += '</body></html>'

            self.wfile.write(output.encode())


    def do_POST(self):

        if self.path.startswith('/imx8'):
            z = self.path.rsplit('/')
            print(z)
            if z[2] == 'CAM_POWER':
               ser=serial.Serial('/dev/ttymxc1')
               ser.write(b'\x81\x01\x04\x00\x03\xFF')
            if z[2] == 'CAM_Zoom':
               if z[3] == 'Stop':
                  ser=serial.Serial('/dev/ttymxc1')
                  ser.write(b'\x81\x01\x04\x07\x00\xFF')
               if z[3] == 'Tele':
                  ser=serial.Serial('/dev/ttymxc1')
                  ser.write(b'\x81\x01\x04\x07\x02\xFF')
               if z[3] == 'Wide':
                  ser=serial.Serial('/dev/ttymxc1')
                  ser.write(b'\x81\x01\x04\x07\x03\xFF')
            if z[2] == 'CAM_MENUKey':
               if z[3] == 'MENU':
                  ser=serial.Serial('/dev/ttymxc1')
                  ser.write(b'\x81\x01\x04\x16\x10\xFF')
               if z[3] == 'ESC':
                  ser=serial.Serial('/dev/ttymxc1')
                  ser.write(b'\x81\x01\x04\x16\x20\xFF')
               if z[3] == 'UP':
                  ser=serial.Serial('/dev/ttymxc1')
                  ser.write(b'\x81\x01\x04\x16\x01\xFF')
               if z[3] == 'DOWN':
                  ser=serial.Serial('/dev/ttymxc1')
                  ser.write(b'\x81\x01\x04\x16\x02\xFF')
               if z[3] == 'RIGHT':
                  ser=serial.Serial('/dev/ttymxc1')
                  ser.write(b'\x81\x01\x04\x16\x08\xFF')
               if z[3] == 'LEFT':
                  ser=serial.Serial('/dev/ttymxc1')
                  ser.write(b'\x81\x01\x04\x16\x04\xFF')
            if z[2] == 'CAM_LR_REVERSE':
               if z[3] == 'ON':
                  ser=serial.Serial('/dev/ttymxc1')
                  ser.write(b'\x81\x01\x04\x61\x02\xFF')
               if z[3] == 'OFF':
                  ser=serial.Serial('/dev/ttymxc1')
                  ser.write(b'\x81\x01\x04\x61\x03\xFF')
            if z[2] == 'CAM_PICTURE_FLIP':
               if z[3] == 'ON':
                  ser=serial.Serial('/dev/ttymxc1')
                  ser.write(b'\x81\x01\x04\x66\x02\xFF')
               if z[3] == 'OFF':
                  ser=serial.Serial('/dev/ttymxc1')
                  ser.write(b'\x81\x01\x04\x66\x03\xFF')
#            if z[2] == 'CAM_WDR':
#               if z[3] == 'ON':
#                  ser=serial.Serial('/dev/ttymxc1')
#                  ser.write(b'\x81\x01\x04\x3D\x02\xFF')
#               if z[3] == 'OFF':
#                  ser=serial.Serial('/dev/ttymxc1')
#                  ser.write(b'\x81\x01\x04\x3D\x03\xFF')
            if z[2] == 'CAM_ICR':
               if z[3] == 'NIGHT':
                  ser=serial.Serial('/dev/ttymxc1')
                  ser.write(b'\x81\x01\x04\x01\x02\xFF')
               if z[3] == 'DAY':
                  ser=serial.Serial('/dev/ttymxc1')
                  ser.write(b'\x81\x01\x04\x01\x03\xFF')
#            self.send_response(200)
#            self.send_header('content-type', 'text/html')
#            self.end_headers()

#            output =''
#            output += '<html><body>'
#            output += '<h1>Task List</h1>'
#            output += '<h3><a href="/tasklist/new">IMX8 SoM</a></h3>'
#            output += '</body></html>'
#            self.wfile.write(output.encode())
            self.send_response(301)
            self.send_header('content-type', 'text/html')
            self.send_header('Location', '/')
            self.end_headers()
        if self.path.endswith('/new'):
            ctype, pdict =  cgi.parse_header(self.headers.get('content-type'))
            pdict['boundary'] = bytes(pdict['boundary'], "utf-8")
            content_len = int(self.headers.get('Content-length'))
            pdict['CONTENT-LENGTH'] = content_len
            if ctype == 'multipart/form-data':
                fields = cgi.parse_multipart(self.rfile, pdict)
                new_task = fields.get('task')
                tasklist.append(new_task[0])

            self.send_response(301)
            self.send_header('content-type', 'text/html')
            self.send_header('Location', '/tasklist')
            self.end_headers()

        if self.path.endswith('/remove'):
            listIDPath = self.path.split('/')[2]
            ctype, pdict =  cgi.parse_header(self.headers.get('content-type'))
            if ctype == 'multipart/form-data':
                list_item = listIDPath.replace('%20', ' ')
                tasklist.remove(list_item)
            self.send_response(301)
            self.send_header('content-type', 'text/html')
            self.send_header('Location', '/tasklist')
            self.end_headers()

def main():
    ser = serial.Serial('/dev/ttymxc1', 9600)  # open serial port
    PORT = 9000
    server_address = ('0.0.0.0', PORT)
    server = HTTPServer(server_address, requestHandler)
    print('Server running on port %s' % PORT)
    server.serve_forever()

if __name__ == '__main__':

    main()
