# OS-assignment-3

This project implements a global chat system where everyone can talk to everyone. Follow the instructions below to run the project.

## Prerequisites

- Ensure you have the necessary tools installed to run `make` commands and execute binary files.

## Getting Started

1. Download the project to your local machine.

2. Navigate to the project root directory in your terminal.

3. Run the following command to build all necessary components:
   ```
   make all
   ```
   This command will call `make all` for all subdirectories.

## Running the Server

- To run the server from directory A or directory C, navigate to the respective directory and execute:
  ```
  ./server
  ```

- **Note:** If you're running the server from directory C, make sure you have already run `make` in directory B. This is not necessary if you've run `make all` at the beginning for all directories as instructed.

## Running the Client

- To run the client, navigate to directory A and execute:
  ```
  ./client
  ```

## Closing the Client

- To close the client, type `exit` or press `Ctrl+C`.

## Closing the Server

- To close the server, press `Ctrl+C`. All connected clients will receive a message that the server is shutting down, and the connection will be closed.

## Troubleshooting

- If you encounter any issues, ensure that you have followed all the instructions correctly and that all necessary components have been built using `make`.
