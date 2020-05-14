# chess_cv

Advances in reinforcement learning have allowed researchers to build algorithms
that can out perform humans at games such as Go, Chess and Dota2. However,
most of this work is internal and does not provide researchers and engineers to
experiment and have the experience of playing against an RL agent using a real
board. In this work, we provide an universal approach to using RL agents such as AlphaZero
with OpenCV to read a real life chess board and feed an input to the
algorithm. We bring the interactive experience to RL agents.

## Installation

The project uses Bazel as a build system [bazel](https://https://bazel.build/) 

```bash
bazel build ..
```

## Usage

Camera Calibration
```bash
bazel run //camera:calibrate
```

Board recognition
```bash
bazel run //boardr:board
```

## Contributing

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License

[MIT](https://choosealicense.com/licenses/mit/)
