import React from 'react';
import { Navigate, Outlet } from 'react-router';

import AuthService from '../services/auth.service';

export default function AuthGuard({ inverted = false, redirect = '/login' }) {
  const isLoggedIn = AuthService.getToken();
  const isAllowed = inverted ? !isLoggedIn : !!isLoggedIn;

  if (!isAllowed) {
    return <Navigate to={redirect} />;
  }

  return (
    <>
      <Outlet />
    </>
  );
}
