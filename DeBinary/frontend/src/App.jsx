import { BrowserRouter, Routes, Route } from "react-router-dom";

import AddDeviceForm from "./components/AddDeviceForm";
import Sidebar from "./components/Sidebar";
import Home from "./pages/Home";
import Devices from "./pages/Devices";

function App() {
  return (
    <>
      <div className="app">
        <BrowserRouter>
          <div className="modal">
            <AddDeviceForm />
          </div>
          <Sidebar />
          <div className="pages">
            <Routes>
              <Route path="/" element={<Home />} />
              <Route path="/device" element={<Devices />} />
            </Routes>
          </div>
        </BrowserRouter>
      </div>
    </>
  );
}

export default App;
