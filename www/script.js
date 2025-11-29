document.addEventListener("DOMContentLoaded", () => {
    const productGrid = document.getElementById("productGrid");
    const searchBar = document.getElementById("searchBar");
    const totalTimeEl = document.getElementById("totalTime");
    const statusTimeEl = document.getElementById("statusTime");
    const statusItemsEl = document.getElementById("statusItems");
    const cartCountEl = document.getElementById("cartCount");
    const toast = document.getElementById("toast");
    const rarityFilter = document.getElementById("rarityFilter");
    const sortSelect = document.getElementById("sortSelect");
    const dailyQuestEl = document.getElementById("dailyQuest");
    const canvas = document.getElementById("particleCanvas");

    const CART_KEY = "cartItems";

    const products = [
        {
            id: 1,
            name: "Aprovat en SIOP",
            rarity: "common",
            timeMinutes: 120,
            description: "Paquet d'estudi garantit per superar SIOP sense perdre el cap.",
            img: "images/AprovatenSIOP.png"
        },
        {
            id: 2,
            name: "Matrícula d'honor en SIOP",
            rarity: "mythic",
            timeMinutes: 330,
            description: "Benvingut al panteó dels alumnes llegendaris.",
            img: "images/MatriculadhonorenSIOP.png"
        },
        {
            id: 3,
            name: "Bolígraf Arcà",
            rarity: "common",
            timeMinutes: 100 * 60,
            description: "Escriu rúniques infinites sense quedar-se sense tinta.",
            img: "images/boligraf.png"
        },
        {
            id: 4,
            name: "Apple MacTime M3",
            rarity: "rare",
            timeMinutes: 3000,
            description: "Portàtil forjat al pla de Cupertino Prime.",
            img: "images/AppleMacTimeM3.png"
        },
        {
            id: 5,
            name: "Glock 18",
            rarity: "rare",
            timeMinutes: 4800,
            description: "Replica de pràctica per a duels temporals controlats.",
            img: "images/Glock18.png"
        },
        {
            id: 6,
            name: "Ampolla d'acer encantada",
            rarity: "uncommon",
            timeMinutes: 45,
            description: "Manté l'ixir de cafè calent durant jornades eternes.",
            img: "images/Ampolladaiguadacerinoxidable.png"
        },
        {
            id: 7,
            name: "Francotirador Barret M82",
            rarity: "mythic",
            timeMinutes: 7200,
            description: "Peça de col·lecció per a mercenaris del temps.",
            img: "images/FrancotiradorBarretM82.png"
        },
        {
            id: 8,
            name: "Yamaha WaveRunner VX23",
            rarity: "mythic",
            timeMinutes: 12000,
            description: "Moto aquàtica infusa amb essència planar.",
            img: "images/YamahaWaveRunnerVX23.png"
        },
        {
            id: 9,
            name: "Glock 20",
            rarity: "rare",
            timeMinutes: 5400,
            description: "Equilibri perfecte entre control i potència rúnica.",
            img: "images/Glock20.png"
        },
        {
            id: 10,
            name: "Suspés remot en SIOP",
            rarity: "uncommon",
            timeMinutes: 180,
            description: "Servei de suspensió estratègica per guanyar temps extra.",
            img: "images/SuspesremotenSIOP.png"
        },
        {
            id: 11,
            name: "Pastilles de cafeïna",
            rarity: "common",
            timeMinutes: 30,
            description: "Efecte instantani per no adormir-te en cap ritual.",
            img: "images/Pastillesdecafeina.png"
        },
        {
            id: 12,
            name: "Rellotge bàsic",
            rarity: "common",
            timeMinutes: 15,
            description: "Controla cada segon sacrificat en els teus intercanvis.",
            img: "images/Rellotgebasic.png"
        },
        {
            id: 13,
            name: "Drac del Vòrtex",
            rarity: "mythic",
            timeMinutes: 9600,
            description: "Mascota dracònica que vigila el teu inventari dimensional.",
            img: "images/dragon.png"
        },
        {
            id: 14,
            name: "Poció d'Amor Temporal",
            rarity: "uncommon",
            timeMinutes: 75,
            description: "Elixir rosat que enamora fins i tot els guardians del temps.",
            img: "images/poscimadelamor.png"
        },
        {
            id: 15,
            name: "Orbe Gemini QH9",
            rarity: "rare",
            timeMinutes: 420,
            description: "Artefacte generatiu que duplica qualsevol relíquia una vegada al dia.",
            img: "images/Gemini_Generated_Image_qh9c21qh9c21qh9c.png"
        }
    ];

    const filters = {
        search: "",
        rarity: "all",
        sort: "default"
    };

    let cart = loadCart();
    let toastTimeout = null;

    function loadCart() {
        const stored = JSON.parse(localStorage.getItem(CART_KEY));
        if (Array.isArray(stored)) return stored;

        const legacyProducts = JSON.parse(localStorage.getItem("cartProducts"));
        if (Array.isArray(legacyProducts) && legacyProducts.length) {
            const normalized = [];
            legacyProducts.forEach((legacy) => {
                const product = products.find((p) => p.name === legacy.name);
                if (!product) return;
                const existing = normalized.find((item) => item.id === product.id);
                if (existing) existing.quantity += 1;
                else normalized.push({ id: product.id, quantity: 1 });
            });
            localStorage.setItem(CART_KEY, JSON.stringify(normalized));
            localStorage.removeItem("cartProducts");
            localStorage.removeItem("cartTotalTime");
            return normalized;
        }
        return [];
    }

    function saveCart() {
        localStorage.setItem(CART_KEY, JSON.stringify(cart));
        const totalMinutes = calculateCartMinutes();
        const { h, m } = splitTime(totalMinutes);
        localStorage.setItem("cartTotalTime", JSON.stringify({ h, m }));
        localStorage.removeItem("cartProducts");
        updateTotals();
    }

    function calculateCartMinutes() {
        return cart.reduce((acc, item) => {
            const product = products.find((p) => p.id === item.id);
            return product ? acc + product.timeMinutes * item.quantity : acc;
        }, 0);
    }

    function splitTime(minutes) {
        return { h: Math.floor(minutes / 60), m: minutes % 60 };
    }

    function formatTime(minutes) {
        const { h, m } = splitTime(minutes);
        return `${h}h ${m}m`;
    }

    function updateTotals() {
        const minutes = calculateCartMinutes();
        const formatted = formatTime(minutes);
        totalTimeEl.textContent = formatted;
        statusTimeEl.textContent = formatted;
        const count = cart.reduce((acc, item) => acc + item.quantity, 0);
        cartCountEl.textContent = count;
        statusItemsEl.textContent = `${count} ${count === 1 ? "objecte" : "objectes"}`;
    }

    function renderProducts() {
        if (!productGrid) return;
        productGrid.innerHTML = "";
        const fragment = document.createDocumentFragment();

        const filtered = products
            .filter((product) => {
                const matchesSearch = product.name.toLowerCase().includes(filters.search);
                const matchesRarity = filters.rarity === "all" || product.rarity === filters.rarity;
                return matchesSearch && matchesRarity;
            })
            .sort((a, b) => {
                if (filters.sort === "asc") return a.timeMinutes - b.timeMinutes;
                if (filters.sort === "desc") return b.timeMinutes - a.timeMinutes;
                return a.id - b.id;
            });

        if (!filtered.length) {
            const empty = document.createElement("div");
            empty.className = "empty-state";
            empty.innerHTML = `<p>No hem trobat cap producte que coincideixi. Prova de canviar la cerca.</p>`;
            productGrid.appendChild(empty);
            return;
        }

        filtered.forEach((product) => fragment.appendChild(createProductCard(product)));
        productGrid.appendChild(fragment);
    }

    function createProductCard(product) {
        const article = document.createElement("article");
        article.className = `product rarity-${product.rarity}`;
        article.innerHTML = `
            <div class="rarity-chip">${product.rarity.toUpperCase()}</div>
            <img src="${product.img}" alt="${product.name}">
            <h3>${product.name}</h3>
            <p class="description">${product.description}</p>
            <div class="time-cost">
                <span>Cost temporal</span>
                <strong>${formatTime(product.timeMinutes)}</strong>
            </div>
            <button class="primary" data-product="${product.id}">Afegir al carret</button>
        `;
        const button = article.querySelector("button");
        button.addEventListener("click", () => addToCart(product.id, product.name));
        return article;
    }

    function addToCart(id, name) {
        const product = products.find((p) => p.id === id);
        if (!product) return;
        const existing = cart.find((item) => item.id === id);
        if (existing) existing.quantity += 1;
        else cart.push({ id, quantity: 1 });
        saveCart();
        showToast(`${name} afegit!`);
    }

    function showToast(message) {
        if (!toast) return;
        toast.textContent = message;
        toast.classList.add("visible");
        clearTimeout(toastTimeout);
        toastTimeout = setTimeout(() => toast.classList.remove("visible"), 2200);
    }

    function initDailyQuest() {
        if (!dailyQuestEl) return;
        const hours = Math.floor(Math.random() * 5) + 1;
        const minutes = Math.floor(Math.random() * 60);
        dailyQuestEl.textContent = formatTime(hours * 60 + minutes);
    }

    function initParticles(target) {
        if (!target) return;
        const ctx = target.getContext("2d");
        let width = 0;
        let height = 0;
        const particles = Array.from({ length: 60 }, () => createParticle());

        function resize() {
            width = target.width = window.innerWidth;
            height = target.height = Math.max(
                document.body.scrollHeight,
                document.documentElement.scrollHeight,
                window.innerHeight
            );
        }

        function createParticle() {
            return {
                x: Math.random() * width,
                y: Math.random() * height,
                radius: Math.random() * 2 + 0.5,
                speedX: (Math.random() - 0.5) * 0.2,
                speedY: Math.random() * 0.5 + 0.15,
                alpha: Math.random() * 0.5 + 0.2,
                hue: 250 + Math.random() * 120
            };
        }

        function draw() {
            const neededHeight = Math.max(
                document.body.scrollHeight,
                document.documentElement.scrollHeight,
                window.innerHeight
            );
            if (neededHeight !== height) {
                height = neededHeight;
                target.height = height;
            }

            ctx.clearRect(0, 0, width, height);
            ctx.globalCompositeOperation = "lighter";
            particles.forEach((particle) => {
                particle.x += particle.speedX;
                particle.y += particle.speedY;
                if (particle.y > height || particle.x < 0 || particle.x > width) {
                    Object.assign(particle, createParticle(), { y: -10 });
                }
                const gradient = ctx.createRadialGradient(
                    particle.x,
                    particle.y,
                    0,
                    particle.x,
                    particle.y,
                    particle.radius * 12
                );
                gradient.addColorStop(0, `hsla(${particle.hue}, 85%, 75%, ${particle.alpha})`);
                gradient.addColorStop(1, "transparent");
                ctx.fillStyle = gradient;
                ctx.beginPath();
                ctx.arc(particle.x, particle.y, particle.radius * 12, 0, Math.PI * 2);
                ctx.fill();
            });
            requestAnimationFrame(draw);
        }

        resize();
        window.addEventListener("resize", resize);
        draw();
    }

    if (searchBar) {
        searchBar.addEventListener("input", (event) => {
            filters.search = event.target.value.trim().toLowerCase();
            renderProducts();
        });
    }

    rarityFilter?.addEventListener("change", (event) => {
        filters.rarity = event.target.value;
        renderProducts();
    });

    sortSelect?.addEventListener("change", (event) => {
        filters.sort = event.target.value;
        renderProducts();
    });

    window.goToCart = function goToCart() {
        window.location.href = "cart.html";
    };

    initDailyQuest();
    initParticles(canvas);
    renderProducts();
    updateTotals();
});

