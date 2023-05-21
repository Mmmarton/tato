import React from 'react';
import { Navigate, Outlet } from 'react-router';

import { getToken } from '../services/auth.service';

export default function AuthGuard({ inverted = false, redirect = '/login' }) {
  const isLoggedIn = getToken();
  const isAllowed = inverted ? !isLoggedIn : !!isLoggedIn;

  console.log({ isAllowed });

  if (!isAllowed) {
    return <Navigate to={redirect} />;
  }

  return (
    <>
      <Outlet />
    </>
  );
}
