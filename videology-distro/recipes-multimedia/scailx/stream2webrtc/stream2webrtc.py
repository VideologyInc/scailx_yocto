#! /usr/bin/env python3
import time
import requests
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler, DirDeletedEvent
import os
from urllib.parse import quote
import argparse
import sys

class StreamFileHandler(FileSystemEventHandler):
    def __init__(self, api_url, api_port):
        self.api_base_url = f"http://{api_url}:{api_port}"

    def on_created(self, event):
        if not event.is_directory:
            print(event)
            try:
                # Get the full file path
                file_path = event.src_path
                filename = os.path.basename(file_path)

                # Construct the API URL with query parameter
                params = {
                    "name": quote(filename),
                    "src": f"exec:gst-launch-1.0 -q unixfdsrc socket-path={file_path} ! videoconvert ! vpuenc_h264 qp-max=30 qp-min=20 ! fdsink"
                }
                url = f"{self.api_base_url}/api/streams"
                response = requests.put(url, params=params)
                print(f"Sending PUT request to: {url}")
                # Send POST request to API
                response = requests.put(url)

                if response.status_code == 200:
                    print(f"Successfully added stream for file: {file_path}")
                else:
                    print(f"Failed to add stream for file: {file_path}. Status code: {response.status_code}")

            except Exception as e:
                print(f"Error processing file {event.src_path}: {str(e)}")
    def on_deleted(self, event):
        if not event.is_directory:
            filename = os.path.basename(event.src_path)
            try:
                # Construct the API URL with query parameter
                params = {
                    "src": quote(filename)
                }
                url = f"{self.api_base_url}/api/streams"
                response = requests.delete(url, params=params)
                print(f"Sending DELETE request to: {url}")
                # Send POST request to API
                response = requests.delete(url)

                if response.status_code == 200:
                    print(f"Successfully deleted stream for file: {filename}")
                else:
                    print(f"Failed to delete stream for file: {filename}. Status code: {response.status_code}")

            except Exception as e:
                print(f"Error processing file {event.src_path}: {str(e)}")

def start_watching(watch_directory, api_url, api_port):
    # Create an observer and event handler
    event_handler = StreamFileHandler(api_url, api_port)
    observer = Observer()

    # Schedule the observer to watch the directory
    observer.schedule(event_handler, watch_directory, recursive=False)

    # Start the observer
    observer.start()

    print(f"Started watching directory: {watch_directory}")
    print(f"API endpoint: http://{api_url}:{api_port}")

    try:
        # Keep the script running
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()
        print("\nStopped watching directory")

    observer.join()

def parse_arguments():
    parser = argparse.ArgumentParser(
        description='Watch directory for new gstreamer unixfd sockets send them to go2rtc API',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument('-d', '--directory',    default='/tmp/stream2webrtc',  help='Directory to watch for new files' )
    parser.add_argument('-H', '--host',         default='localhost',                help='go2rtc API host')
    parser.add_argument('-p', '--port',         default=1984,       type=int,       help='go2rtc API port')
    return parser.parse_args()

def main():
    # Parse command line arguments
    args = parse_arguments()
    if not os.path.exists(args.directory):
        os.makedirs(args.directory)

    # Start watching
    try:
        start_watching(args.directory, args.host, args.port)
    except Exception as e:
        print(f"Error: {str(e)}")
        sys.exit(1)

if __name__ == "__main__":
    main()
