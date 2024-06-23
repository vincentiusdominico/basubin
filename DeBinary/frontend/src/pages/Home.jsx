import { useEffect } from "react";

const Home = () => {
  useEffect(() => {
    const [cusomers, setCustomers] = useState(null);
    const fetchCustomers = async () => {
      const response = await fetch("http://127.0.0.1:8000/api/customer/");
      const json = await response.json();
      if (response.ok) {
        setCustomers(json);
      }
      fetchCustomers();
    };
  }, []);

  return (
    <div className="home">
      <h1>yeyeyey</h1>
      <div className="customers">
        {customers &&
          customers.map((customer) => {
            <p key={customer._id}>{customer.name}</p>;
          })}
      </div>
    </div>
  );
};

export default Home;
