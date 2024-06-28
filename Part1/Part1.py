#!/usr/bin/env python

import vtk
import vtkmodules.vtkInteractionStyle
import vtkmodules.vtkRenderingOpenGL2
from vtkmodules.vtkCommonColor import vtkNamedColors
from vtkmodules.vtkCommonCore import (
    VTK_VERSION_NUMBER,
    vtkVersion
)
from vtkmodules.vtkFiltersCore import (
    vtkFlyingEdges3D,
    vtkMarchingCubes,
    vtkStripper
)
from vtkmodules.vtkFiltersModeling import vtkOutlineFilter
from vtkmodules.vtkIOImage import vtkMetaImageReader
from vtkmodules.vtkRenderingCore import (
    vtkActor,
    vtkCamera,
    vtkPolyDataMapper,
    vtkProperty,
    vtkRenderWindow,
    vtkRenderWindowInteractor,
    vtkRenderer,
    vtkTextActor,
    vtkVolume,
    vtkVolumeProperty
)
from vtkmodules.vtkInteractionWidgets import (
    vtkSliderRepresentation2D,
    vtkSliderWidget,
    vtkTextRepresentation,
    vtkTextWidget
)
from vtkmodules.vtkRenderingVolume import (
    vtkFixedPointVolumeRayCastMapper,

)
from vtkmodules.vtkRenderingVolumeOpenGL2 import vtkOpenGLGPUVolumeRayCastMapper
from vtkmodules.vtkCommonDataModel import vtkPiecewiseFunction
from vtkmodules.vtkRenderingCore import vtkColorTransferFunction

def get_program_parameters():
    import argparse
    description = 'Read a VTK image data file.'
    epilogue = ''''''
    parser = argparse.ArgumentParser(description=description, epilog=epilogue,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('filename', help='FullHead.mhd')
    args = parser.parse_args()
    return args.filename


def main():
    filename = "FullHead.mhd"
    
    # Renderer, Render Window, and Interactor
    colors = vtkNamedColors()
    colors.SetColor('SkinColor', [240, 184, 160, 255])
    colors.SetColor('BackfaceColor', [255, 229, 200, 255])
    colors.SetColor('BkgColor', [51, 77, 102, 255])

    renderer = vtk.vtkRenderer()
    renderWindow = vtk.vtkRenderWindow()
    renderWindow.AddRenderer(renderer)
    renderWindow.SetWindowName('VolumeRendering')
    interactor = vtk.vtkRenderWindowInteractor()
    interactor.SetRenderWindow(renderWindow)
    
    # Reader
    reader = vtk.vtkMetaImageReader()
    reader.SetFileName(filename)
    
    # Iso-surface extraction setup
    use_flying_edges = vtk.vtkVersion.GetVTKMajorVersion() >= 8 and vtk.vtkVersion.GetVTKMinorVersion() >= 90
    skin_extractor = vtk.vtkFlyingEdges3D() if use_flying_edges else vtk.vtkMarchingCubes()
    skin_extractor.SetInputConnection(reader.GetOutputPort())
    skin_extractor.SetValue(0, 500)
    skin_stripper = vtk.vtkStripper()
    skin_stripper.SetInputConnection(skin_extractor.GetOutputPort())
    skin_mapper = vtk.vtkPolyDataMapper()
    skin_mapper.SetInputConnection(skin_stripper.GetOutputPort())
    skin_mapper.ScalarVisibilityOff()
    skin = vtk.vtkActor()
    skin.SetMapper(skin_mapper)
    skin.GetProperty().SetDiffuseColor(colors.GetColor3d('SkinColor'))
    skin.GetProperty().SetSpecular(0.3)
    skin.GetProperty().SetSpecularPower(20)
    skin.GetProperty().SetOpacity(0.5)
    back_prop = vtk.vtkProperty()
    back_prop.SetDiffuseColor(colors.GetColor3d('BackfaceColor'))
    skin.SetBackfaceProperty(back_prop)
    bone_extractor = vtk.vtkFlyingEdges3D() if use_flying_edges else vtk.vtkMarchingCubes()
    bone_extractor.SetInputConnection(reader.GetOutputPort())
    bone_extractor.SetValue(0, 1150)
    bone_stripper = vtk.vtkStripper()
    bone_stripper.SetInputConnection(bone_extractor.GetOutputPort())
    bone_mapper = vtk.vtkPolyDataMapper()
    bone_mapper.SetInputConnection(bone_stripper.GetOutputPort())
    bone_mapper.ScalarVisibilityOff()
    bone = vtk.vtkActor()
    bone.SetMapper(bone_mapper)
    bone.GetProperty().SetDiffuseColor(colors.GetColor3d('Ivory'))
    outline_data = vtk.vtkOutlineFilter()
    outline_data.SetInputConnection(reader.GetOutputPort())
    map_outline = vtk.vtkPolyDataMapper()
    map_outline.SetInputConnection(outline_data.GetOutputPort())
    outline = vtk.vtkActor()
    outline.SetMapper(map_outline)
    outline.GetProperty().SetColor(colors.GetColor3d('Black'))

    # Volume rendering setup
    iso1 = 500.0
    iso2 = 1150.0
    ray_step_size = 0.5
    mapper = vtk.vtkOpenGLGPUVolumeRayCastMapper()
    mapper.SetInputConnection(reader.GetOutputPort())
    mapper.AutoAdjustSampleDistancesOff()
    mapper.SetSampleDistance(ray_step_size)
    mapper.SetBlendModeToIsoSurface()
    colorTransferFunction = vtk.vtkColorTransferFunction()
    colorTrans1 = colors.GetColor3d("flesh")
    colorTrans2 = colors.GetColor3d("ivory")
    colorTransferFunction.AddRGBPoint(iso1, colorTrans1[0], colorTrans1[1], colorTrans1[2])
    colorTransferFunction.AddRGBPoint(iso2, colorTrans2[0], colorTrans2[1], colorTrans2[2])
    scalarOpacity = vtk.vtkPiecewiseFunction()
    scalarOpacity.AddPoint(iso1, .3)
    scalarOpacity.AddPoint(iso2, 0.6)
    volumeProperty = vtk.vtkVolumeProperty()
    volumeProperty.ShadeOn()
    volumeProperty.SetInterpolationTypeToLinear()
    volumeProperty.SetColor(colorTransferFunction)
    volumeProperty.SetScalarOpacity(scalarOpacity)
    volumeProperty.GetIsoSurfaceValues().SetValue(0, iso1)
    volumeProperty.GetIsoSurfaceValues().SetValue(1, iso2)
    volume = vtk.vtkVolume()
    volume.SetMapper(mapper)
    volume.SetProperty(volumeProperty)

    # Initial setup for rendering
    renderer.AddActor(outline)
    renderer.AddActor(skin)
    renderer.AddActor(bone)
    renderer.SetBackground(colors.GetColor3d('BkgColor'))  # Set background color
    renderWindow.SetSize(800, 600)
    renderWindow.SetWindowName("VolumeRendering")

    # Initial camera setup
    a_camera = vtk.vtkCamera()
    a_camera.SetViewUp(0, 0, -1)
    a_camera.SetPosition(0, -1, 0)
    a_camera.SetFocalPoint(0, 0, 0)
    a_camera.Azimuth(30.0)
    a_camera.Elevation(30.0)
    a_camera.Dolly(1.5)
    renderer.SetActiveCamera(a_camera)
    renderer.ResetCamera()
    renderer.ResetCameraClippingRange()

    # Slider for skin opacity
    sliderRep = vtk.vtkSliderRepresentation2D()
    sliderRep.SetMinimumValue(0.0)
    sliderRep.SetMaximumValue(1.0)
    sliderRep.SetValue(skin.GetProperty().GetOpacity())
    sliderRep.SetTitleText("Skin Opacity")
    sliderRep.GetPoint1Coordinate().SetCoordinateSystemToNormalizedDisplay()
    sliderRep.GetPoint1Coordinate().SetValue(0.1, 0.1)
    sliderRep.GetPoint2Coordinate().SetCoordinateSystemToNormalizedDisplay()
    sliderRep.GetPoint2Coordinate().SetValue(0.8, 0.1)
    sliderRep.GetSliderProperty().SetColor(colors.GetColor3d("Green"))
    sliderRep.GetTitleProperty().SetColor(colors.GetColor3d("AliceBlue"))
    sliderRep.GetLabelProperty().SetColor(colors.GetColor3d("AliceBlue"))
    sliderRep.GetSelectedProperty().SetColor(colors.GetColor3d("DeepPink"))
    sliderRep.GetTubeProperty().SetColor(colors.GetColor3d("MistyRose"))
    sliderRep.GetCapProperty().SetColor(colors.GetColor3d("Yellow"))
    sliderRep.SetSliderLength(0.05)
    sliderRep.SetSliderWidth(0.025)
    sliderRep.SetEndCapLength(0.02)
    sliderWidget = vtk.vtkSliderWidget()
    sliderWidget.SetRepresentation(sliderRep)
    sliderWidget.SetInteractor(interactor)
    sliderWidget.SetAnimationModeToAnimate()
    sliderWidget.EnabledOn()
    opacitySkinSliderCallback = vtkSliderCallback(skin.GetProperty())
    sliderWidget.AddObserver("InteractionEvent", opacitySkinSliderCallback)

    # Variable to track the current rendering mode
    current_mode = [0]  # 0 for iso-surface, 1 for volume rendering

    # Callback function to switch rendering modes
    def keypress_callback(obj, event):
        nonlocal iso1, iso2, ray_step_size, current_mode

        key = obj.GetKeySym()

        if key == "Right":
            ray_step_size += 0.1
        elif key == "Left":
            ray_step_size -= 0.1
        elif key == "1":  # Increase opacity for iso1
            scalarOpacity.AddPoint(iso1, scalarOpacity.GetValue(iso1) + 0.1)
        elif key == "2":  # Decrease opacity for iso1
            scalarOpacity.AddPoint(iso1, scalarOpacity.GetValue(iso1) - 0.1)
        elif key == "3":  # Increase opacity for iso2
            scalarOpacity.AddPoint(iso2, scalarOpacity.GetValue(iso2) + 0.1)
        elif key == "4":  # Decrease opacity for iso2
            scalarOpacity.AddPoint(iso2, scalarOpacity.GetValue(iso2) - 0.1)
        elif key == "v":  # Switch to volume rendering mode
            if current_mode[0] == 0:
                renderer.RemoveViewProp(skin)
                renderer.RemoveViewProp(bone)
                renderer.AddVolume(volume)
                sliderWidget.SetEnabled(False)  # Disable slider in volume rendering mode
                current_mode[0] = 1
        elif key == "i":  # Switch to iso-surface extraction mode
            if current_mode[0] == 1:
                renderer.RemoveVolume(volume)
                renderer.AddActor(skin)
                renderer.AddActor(bone)
                sliderWidget.SetEnabled(True)  # Enable slider in iso-surface mode
                current_mode[0] = 0

        # Clamp opacity values between 0 and 1
        scalarOpacity.ClampingOn()
        scalarOpacity.AddPoint(iso1, max(0.0, min(1.0, scalarOpacity.GetValue(iso1))))
        scalarOpacity.AddPoint(iso2, max(0.0, min(1.0, scalarOpacity.GetValue(iso2))))

        # Update iso-values
        colorTransferFunction.RemoveAllPoints()
        colorTransferFunction.AddRGBPoint(iso2, colorTrans2[0], colorTrans2[1], colorTrans2[2])
        colorTransferFunction.AddRGBPoint(iso1, colorTrans1[0], colorTrans1[1], colorTrans1[2])

        # Update ray step size
        mapper.SetSampleDistance(ray_step_size)

        # Render the window to reflect changes
        renderWindow.Render()

    interactor.AddObserver("KeyPressEvent", keypress_callback)
    interactor.Initialize()
    renderWindow.Render()
    interactor.Start()

# Slider callback class
class vtkSliderCallback:
    def __init__(self, prop):
        self.prop = prop

    def __call__(self, caller, ev):
        sliderWidget = caller
        value = sliderWidget.GetRepresentation().GetValue()
        self.prop.SetOpacity(value)

def vtk_version_ok(major, minor, build):
    """
    Check the VTK version.
    """
    return VTK_VERSION_NUMBER >= (major * 10000000000 + minor * 100000000 + build * 1000000)

if __name__ == '__main__':
    main()
