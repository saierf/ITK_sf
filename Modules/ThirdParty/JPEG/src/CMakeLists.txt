set(ITK3P_INSTALL_EXPORT_NAME "${ITKJPEG-targets}")
set(ITK3P_INSTALL_INCLUDE_DIR "${ITKJPEG_INSTALL_INCLUDE_DIR}")
set(ITK3P_INSTALL_RUNTIME_DIR "${ITKJPEG_INSTALL_RUNTIME_DIR}")
set(ITK3P_INSTALL_LIBRARY_DIR "${ITKJPEG_INSTALL_LIBRARY_DIR}")
set(ITK3P_INSTALL_ARCHIVE_DIR "${ITKJPEG_INSTALL_ARCHIVE_DIR}")
if (ITK_USE_JPEG_TURBO)
    add_subdirectory(itkjpeg-turbo)
else()
    add_subdirectory(itkjpeg)
endif()
itk_module_target(itkjpeg NO_INSTALL)
