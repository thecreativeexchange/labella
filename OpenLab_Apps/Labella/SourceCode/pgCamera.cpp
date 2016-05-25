////////////////////////////////////////////////////////////////////////////////
// pgCamera.cpp

#include "ApConfig.h"
#include "BtTime.h"
#include "MtMatrix3.h"
#include "pgCamera.h"
#include "ShTouch.h"
#include "UiKeyboard.h"
#include "RsUtil.h"
#include "ShHMD.h"
#include "FsFile.h"
#include "HlKeyboard.h"
#include "ShIMU.h"
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////
// Init

void pgCamera::Init()
{
	BtFloat width = (BtFloat)RsUtil::GetWidth();
	BtFloat height = (BtFloat)RsUtil::GetHeight();

	m_camera = RsCamera(0.1f, 10000.0f, width / height, RsViewport(0, 0, (BtU32)width, (BtU32)height), MtDegreesToRadians(60.0f));
	m_camera.SetDimension(RsUtil::GetDimension());
	m_camera.SetPerspective(BtTrue);

	m_cameraData.m_v3Position = MtVector3(0, 0, 0);
	m_cameraData.m_m3Rotation.SetIdentity();

	m_speed = 100.0f;
	m_cameraData.m_pitch = 0;
	m_cameraData.m_yaw = 0;
	m_isFlyCam = BtTrue;
}

////////////////////////////////////////////////////////////////////////////////
// SetDimension

void pgCamera::SetDimension(const MtVector2 &v2Dimension)
{
	BtFloat width = v2Dimension.x;
	BtFloat height = v2Dimension.y;

	BtFloat fieldOfView = m_camera.FieldOfView();
	BtFloat nearPlane = m_camera.NearPlane();
	BtFloat farPlane = m_camera.FarPlane();
	m_camera = RsCamera(nearPlane, farPlane, width / height, RsViewport(0, 0, (BtU32)width, (BtU32)height), fieldOfView);
	m_camera.SetDimension(v2Dimension);
	m_camera.SetPerspective(BtTrue);
}

////////////////////////////////////////////////////////////////////////////////
// IsFlyCam

BtBool pgCamera::IsFlyCam()
{
	return m_isFlyCam;
}

////////////////////////////////////////////////////////////////////////////////
// SetPosition

void pgCamera::SetPosition(const MtVector3 &v3Position)
{
	m_cameraData.m_v3Position = v3Position;
}

////////////////////////////////////////////////////////////////////////////////
// GetPosition

MtVector3 pgCamera::GetPosition()
{
	return m_cameraData.m_v3Position;
}

////////////////////////////////////////////////////////////////////////////////
// SetSpeed

void pgCamera::SetSpeed(BtFloat speed)
{
	m_speed = speed;
}

////////////////////////////////////////////////////////////////////////////////
// Update

void pgCamera::Update()
{
	if (ApConfig::IsWin())
	{
		static BtBool isLoaded = BtFalse;
		if (isLoaded == BtFalse)
		{
			// If we have a previously saved version of the camera then load
			FsFile file;
			BtChar filename[64];
			sprintf(filename, "%s%s", ApConfig::GetDocuments(), "camera.txt");
			file.Open(filename, FsMode_Read);
			if (file.IsOpen())
			{
				file.Read(m_cameraData);
				file.Close();
			}
			isLoaded = BtTrue;
		}
	}

	if (UiKeyboard::pInstance()->IsPressed(UiKeyCode_F3))
	{
		m_isFlyCam = !m_isFlyCam;
	}


	for (BtU32 i = 1; i < MaxTouches; i += 2)
	{
		if (ShTouch::IsHeld(i) || m_isFlyCam)
		{
			BtFloat speed = BtTime::GetTick() * 10.0f;

			if (UiKeyboard::pInstance()->IsHeld(UiKeyCode_LSHIFT))
			{
				speed = speed * 10.0f;
			}
			if (UiKeyboard::pInstance()->IsHeld(UiKeyCode_W))
			{
				MoveForward(speed);
			}
			if (UiKeyboard::pInstance()->IsHeld(UiKeyCode_S))
			{
				MoveBackward(speed);
			}
			if (UiKeyboard::pInstance()->IsHeld(UiKeyCode_D))
			{
				MoveRight(speed);
			}
			if (UiKeyboard::pInstance()->IsHeld(UiKeyCode_A))
			{
				MoveLeft(speed);
			}
		}
	}


	if (ShTouch::IsHeld(1))
	{
        BtBool onScreen = BtTrue;//ShTouch::OnScreen(1);
		if (onScreen)
		{
			for (BtU32 i = 0; i < MaxTouches; i += 2)
			{
				// Rotate the camera
				MtVector2 v2MouseDirection = ShTouch::GetMovement(i);
				BtFloat speed = BtTime::GetTick() * 0.1f;
				MtMatrix3 m3Rotate;
				m3Rotate.SetRotationY(v2MouseDirection.x * -speed);
				m_cameraData.m_m3Rotation = m3Rotate * m_cameraData.m_m3Rotation;
				m3Rotate.SetRotationX(v2MouseDirection.y * speed);
				m_cameraData.m_m3Rotation = m_cameraData.m_m3Rotation * m3Rotate;
			}
		}
	}

	static BtBool isCursorKeys = BtFalse;

	// Rotate the camera
	BtFloat speed = BtTime::GetTick();

	if (UiKeyboard::pInstance()->IsHeld(UiKeyCode_LSHIFT))
	{
		if (UiKeyboard::pInstance()->IsHeld(UiKeyCode_LEFT))
		{
			isCursorKeys = BtTrue;
			m_cameraData.m_yaw += speed;
		}
		if (UiKeyboard::pInstance()->IsHeld(UiKeyCode_RIGHT))
		{
			isCursorKeys = BtTrue;
			m_cameraData.m_yaw -= speed;
		}
		if (UiKeyboard::pInstance()->IsHeld(UiKeyCode_UP))
		{
			isCursorKeys = BtTrue;
			m_cameraData.m_pitch -= speed;
		}
		if (UiKeyboard::pInstance()->IsHeld(UiKeyCode_DOWN))
		{
			isCursorKeys = BtTrue;
			m_cameraData.m_pitch += speed;
		}
	}

	if (!ApConfig::IsWin())
	{
		// Support a landscape quaternion
		MtMatrix3 m_m3Rotation;
		MtQuaternion quaternion = ShIMU::GetQuaternion(0);
		quaternion.x = quaternion.x;
		quaternion.y = quaternion.y;
		quaternion.z = -quaternion.z;

		m_m3Rotation = MtMatrix3(quaternion);
		//m_m3Rotation = m_m3Rotation.GetInverse();

		MtMatrix3 m3RotateY;
		m3RotateY.SetRotationY(MtDegreesToRadians(90.0f));

		MtMatrix3 m3RotateZ;
		m3RotateZ.SetRotationZ(MtDegreesToRadians(-90.0f));

		m_m3Rotation = m3RotateY * m_m3Rotation;
		m_m3Rotation = m3RotateZ * m_m3Rotation;
		m_m3Rotation = m_m3Rotation * m3RotateZ;

		m_camera.SetRotation(m_m3Rotation);

	}
	else if (ShHMD::IsHMD())
	{
		MtMatrix3 m3RotateY;
		m3RotateY.SetRotationX(m_cameraData.m_pitch);
		MtMatrix3 m3RotateX;
		m3RotateX.SetRotationY(m_cameraData.m_yaw);
		m_cameraData.m_m3Rotation = m3RotateX * m3RotateY;

		// Cache the ShHMD rotation
		MtQuaternion quaternion = ShHMD::GetQuaternion();

		// Set the IMU rotation
		MtMatrix4 m4FinalRotation = m_cameraData.m_m3Rotation * MtMatrix3(quaternion);

		// Set the rotation
		m_camera.SetRotation(m4FinalRotation);
	}
	else
	{
		if (isCursorKeys)
		{
			MtMatrix3 m3RotateY;
			m3RotateY.SetRotationX(m_cameraData.m_pitch);
			MtMatrix3 m3RotateX;
			m3RotateX.SetRotationY(m_cameraData.m_yaw);
			m_cameraData.m_m3Rotation = m3RotateX * m3RotateY;
		}

		// Set the rotation
		m_camera.SetRotation(m_cameraData.m_m3Rotation);
	}

	// Set the position
	m_camera.SetPosition(m_cameraData.m_v3Position);

	// Update the camera
	m_camera.Update();

	if (UiKeyboard::pInstance()->IsPressed(SaveCameraKey))
	{
		FsFile file;
		BtChar filename[64];
		sprintf(filename, "%s%s", ApConfig::GetDocuments(), "camera.txt");
		file.Open(filename, FsMode_Write);
		if (file.IsOpen())
		{
			file.Write(m_cameraData);
			file.Close();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// SetFarPlane

void pgCamera::SetFarPlane(BtFloat farPlane)
{
	m_camera.FarPlane(farPlane);
}

////////////////////////////////////////////////////////////////////////////////
// MoveUpward

void pgCamera::MoveRight(BtFloat speed)
{
	speed = BtTime::GetTick() * speed;

	MtMatrix3 m3Rotation = m_camera.GetRotation();
	m_cameraData.m_v3Position += m3Rotation.Col0() * speed;
}

////////////////////////////////////////////////////////////////////////////////
// MoveDownward

void pgCamera::MoveLeft(BtFloat speed)
{
	speed = BtTime::GetTick() * -speed;

	MtMatrix3 m3Rotation = m_camera.GetRotation();
	m_cameraData.m_v3Position += m3Rotation.Col0() * speed;
}

////////////////////////////////////////////////////////////////////////////////
// MoveForward

void pgCamera::MoveForward(BtFloat speed)
{
	speed = BtTime::GetTick() * speed;

	MtMatrix3 m3Rotation = m_camera.GetRotation();
	m_cameraData.m_v3Position += m3Rotation.Col2() * speed;
}

////////////////////////////////////////////////////////////////////////////////
// MoveBackward

void pgCamera::MoveBackward(BtFloat speed)
{
	speed = BtTime::GetTick() * speed;

	MtMatrix3 m3Rotation = m_camera.GetRotation();
	m_cameraData.m_v3Position -= m3Rotation.Col2() * speed;
}

////////////////////////////////////////////////////////////////////////////////
// StrafeRight

void pgCamera::StrafeRight(BtFloat speed)
{
	speed = BtTime::GetTick() * speed;
	MtMatrix3 m3Rotation = m_camera.GetRotation();
	m_cameraData.m_v3Position += m3Rotation.Col0() * speed;
}

////////////////////////////////////////////////////////////////////////////////
// StrafeLeft

void pgCamera::StrafeLeft(BtFloat speed)
{
	speed = BtTime::GetTick() * speed;
	MtMatrix3 m3Rotation = m_camera.GetRotation();
	m_cameraData.m_v3Position -= m3Rotation.Col0() * speed;
}

////////////////////////////////////////////////////////////////////////////////
// GetCamera

RsCamera &pgCamera::GetCamera()
{
	return m_camera;
}
