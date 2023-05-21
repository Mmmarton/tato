import './App.scss';

import { BrowserRouter, Route, Routes } from 'react-router-dom';

import AuthGuard from './components/AuthGuard';
import HomePage from './pages/home/HomePage';
import LoginPage from './pages/login/LoginPage';

function App() {
  return (
    <BrowserRouter>
      <Routes>
        <Route element={<AuthGuard inverted redirect='/' />}>
          <Route exact path='/login' element={<LoginPage />} />
        </Route>

        <Route element={<AuthGuard />}>
          <Route path='/*' element={<HomePage />} />
        </Route>
      </Routes>
    </BrowserRouter>
  );
}

export default App;
