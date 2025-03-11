<script lang="ts">
	import '../app.css';
	import { base } from '$app/paths';
</script>

<svelte:head>
	<!-- Essential Meta Tags -->
	<title>BikeTitans | Bike Rack Availability Tracker</title>
	<meta charset="utf-8" />
	<meta name="viewport" content="width=device-width, initial-scale=1" />
	<meta
		name="description"
		content="BikeTitans helps you find available bike racks in your area. Track open slots and full bike racks in real-time."
	/>

	<!-- SEO Meta Tags -->
	<meta name="robots" content="index, follow" />
	<meta
		name="keywords"
		content="bike racks, bicycle parking, bike parking availability, cycling infrastructure, bike dock tracker"
	/>
	<meta name="author" content="BikeTitans" />
	<link rel="canonical" href="https://biketitans.com{base}" />

	<!-- Open Graph / Facebook Meta Tags -->
	<meta property="og:type" content="website" />
	<meta property="og:url" content="https://biketitans.com{base}" />
	<meta property="og:title" content="BikeTitans | Find Available Bike Racks Near You" />
	<meta
		property="og:description"
		content="Locate open bike racks and check availability in real-time with BikeTitans tracking service."
	/>
	<meta property="og:image" content="{base}/favicon.png" />

	<!-- Twitter Meta Tags -->
	<meta name="twitter:card" content="summary_large_image" />
	<meta name="twitter:url" content="https://biketitans.com{base}" />
	<meta name="twitter:title" content="BikeTitans | Find Available Bike Racks Near You" />
	<meta
		name="twitter:description"
		content="Locate open bike racks and check availability in real-time with BikeTitans tracking service."
	/>
	<meta name="twitter:image" content="{base}/favicon.png" />

	<!-- Favicon -->
	<link rel="icon" type="image/png" href="{base}/favicon.png" />
	<link rel="apple-touch-icon" sizes="180x180" href="{base}/favicon.png" />
</svelte:head>

<div class="flex min-h-screen flex-col">
	<nav class="bg-slate-800 py-1.5 text-white shadow-md">
		<div class="container mx-auto px-4 py-3">
			<div class="flex items-center justify-between">
				<a href="{base}/" class="text-xl font-bold">BikeTitans</a>
				<div class="space-x-6">
					<a href="{base}/" class="transition-colors hover:text-blue-300">Home</a>
					<a href="{base}/about" class="transition-colors hover:text-blue-300">About</a>
					<a href="{base}/contact" class="transition-colors hover:text-blue-300">Contact</a>
				</div>
			</div>
		</div>
	</nav>

	<main class="container mx-auto flex-grow px-4 py-8">
		<slot />
	</main>

	<footer class="bg-slate-800 py-6 text-white">
		<div class="container mx-auto px-4 text-center">
			<p>© {new Date().getFullYear()} BikeTitans. All rights reserved.</p>
		</div>
	</footer>
</div>
