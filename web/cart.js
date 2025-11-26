// References
const totalCart = document.getElementById('totalCart');
const productList = document.getElementById('productList');

// Load data from localStorage
let totalTime = JSON.parse(localStorage.getItem('cartTotalTime')) || {h:0, m:0};
let cartProducts = JSON.parse(localStorage.getItem('cartProducts')) || [];

// Display total time
totalCart.textContent = `Total acumulat: ${totalTime.h}h ${totalTime.m}m`;

// Display products
productList.innerHTML = "";
if(cartProducts.length > 0){
    const ul = document.createElement('ul');
    cartProducts.forEach(product => {
        const li = document.createElement('li');
        li.textContent = `${product.name} - ${product.time.h}h ${product.time.m}m`;
        ul.appendChild(li);
    });
    productList.appendChild(ul);
} else {
    productList.textContent = "El carret està buit.";
}

// Pay button
document.getElementById('payButton').addEventListener('click', () => {
    alert("Funcionalitat de pagament no implementada!");
});

// Go back
window.goBack = function() {
    window.location.href = "index.html";
};

