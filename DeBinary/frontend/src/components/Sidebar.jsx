import { Link } from "react-router-dom";

import logo from "../assets/logo.png";

const Sidebar = () => {
  return (
    <aside>
      <Link className="logo-container">
        <div className="logo">
          <img src={logo} alt="logo" />
        </div>
      </Link>
      <nav>
        <ul>
          <li>
            <Link className="icon-container" to="/">
              <span className="icon">
                <i>
                  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24">
                    <path
                      strokeWidth="2"
                      d="M3.5 11.1213V19C3.5 19.8284 4.17157 20.5 5 20.5H8.5C9.32843 20.5 10 19.8284 10 19V14.75C10 14.3358 10.3358 14 10.75 14H13.25C13.6642 14 14 14.3358 14 14.75V19C14 19.8284 14.6716 20.5 15.5 20.5H19C19.8284 20.5 20.5 19.8284 20.5 19V11.1213C20.5 10.7235 20.342 10.342 20.0607 10.0607L13.0607 3.06066C12.4749 2.47487 11.5251 2.47487 10.9393 3.06066L3.93934 10.0607C3.65804 10.342 3.5 10.7235 3.5 11.1213Z"
                    />
                  </svg>
                </i>
              </span>
              <span className="alt">Home</span>
            </Link>
            <Link className="icon-container" to="/">
              <span className="icon">
                <i>
                  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24">
                    <path strokeLinecap="round" d="M12 8V16" />
                    <path strokeLinecap="round" d="M8 12L16 12" />
                    <circle cx="10" cy="10" r="10" transform="matrix(1 0 0 -1 2 22)" />
                  </svg>
                </i>
              </span>
              <span className="alt">Add Devices</span>
            </Link>
            <Link className="icon-container" to="/">
              <span className="icon">
                <i>
                  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24">
                    <path strokeLinecap="round" d="M12 8V16" />
                    <path strokeLinecap="round" d="M8 12L16 12" />
                    <circle cx="10" cy="10" r="10" transform="matrix(1 0 0 -1 2 22)" />
                  </svg>
                </i>
              </span>
              <span className="alt">Add Devices</span>
            </Link>
          </li>
        </ul>
      </nav>
      {/* <section>
        <h3>Informasi Tambahan</h3>
        <p>Informasi tambahan atau widget dapat ditempatkan di sini.</p>
        </section> */}
    </aside>
  );
};

export default Sidebar;
