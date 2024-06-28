# Part 1 Documentation

In our chosen volume, there are two extracted iso-surfaces:
1. **Skin** (iso-value: 500.0)
2. **Bone** (iso-value: 1150.0)

## Program Overview
1. **Part1.py** - Program for Achieving Ray Marching Rendering and Iso-surface Extraction
2. **Transfer.py** - Program for Setting Up Transfer Function Using Appropriate Colors

## Documentation of Part1.py

### Part1.py (./Part1/Part1.py)
**Dataset Used**: FullHead.mhd (./Part1/FullHead.mhd)

> [!NOTE] <p>To run the program, you need to prompt `python Part1.py FullHead.mhd` in the terminal. If you would like to read other .mhd datasets, you could edit Part1.py to change the chosen file address to the desired one.</p>
> 

### Part1.py (./Part1/headsq/Part1.py)
**Dataset Used**: {quarter.1, quarter.2, quarter.3, ...., quarter.93} (./Part1/headsq/...)

> [!NOTE] <p>Before running the code, you need to place the program in the folder where the used dataset is located.
> If you would like to read another dataset, you could edit Part1.py to change the dataset name to the desired one and modify the number of datasets according to your desired dataset.</p>

### User Control

#### Switching Rendering Modes
- `v` (Keyboard): Switch to Marching Ray Rendering Mode
- `i` (Keyboard): Switch to Isosurface Extraction Mode

#### Marching Ray Rendering Mode
1. **Modifying Ray Step Size**
   - `Right` (Keyboard): Increase ray step size
   - `Left` (Keyboard): Decrease ray step size

2. **Modifying Opacity Transfer Function**
   - **Skin**
     - `1` (Keyboard): Increase Opacity
     - `2` (Keyboard): Decrease Opacity
   - **Bone**
     - `3` (Keyboard): Increase Opacity
     - `4` (Keyboard): Decrease Opacity

#### Isosurface Extraction Mode
1. **Modifying Extracted Isosurface Value**
   - `Up` (Keyboard): Increase Isosurface Value
   - `Down` (Keyboard): Decrease Isosurface Value

2. **Modifying Opacity**
   - `Left` (Slider): Decrease Opacity
   - `Right` (Slider): Increase Opacity

## Documentation of Transfer.py (./Part1/headsq/Transfer.py)

**Dataset Used**: {quarter.1, quarter.2, quarter.3, ...., quarter.93} (./Part1/headsq/...)

> [!NOTE] <p> Before running the code, you need to place the program in the folder where the used dataset is located.
> If you would like to read another dataset, you could edit Transfer.py to change the dataset name to the desired one and modify the number of datasets according to your desired dataset.
> You need to understand the isosurfaces of the dataset you choose, then modify the transfer function according to your requirements.
> To run the program, you need to prompt `python Transfer.py quarter` in the terminal.</p>

